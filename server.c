#define _POSIX_C_SOURCE  200112L

#include <serverUtilities.h>
#include <BST.h>

#define DIR_MODE 0777
#define MAX_FD 64

#define CON 0
#define STR 1
#define RET 2
#define DEL 3
#define DIS 4

#define NUM_TOT 5
#define NUM_OK 4

#define STORE 0
#define RETRIEVE 1

#define INC_TOT(i) LOCK((statTotMtx[i])); statTot[i]++; UNLOCK((statTotMtx[i])); //incrementa statTot[i]
#define INC_OK(i) LOCK((statOkMtx[i])); statOk[i]++; UNLOCK((statOkMtx[i])); //incrementa statOk[i]
#define ADD_DATA(i,len) LOCK((dataSizeMtx[i])); dataSize[i]+=len; UNLOCK((dataSizeMtx[i])); //incrementa dataSize[i]
#define ADD_TASK LOCK(tasksMtx); if(tasks<0) debug fprintf(stderr,"ERROR: negative tasks\n"); tasks++; UNLOCK(tasksMtx); // incrementa tasks
#define REMOVE_TASK LOCK(tasksMtx); tasks--; if(tasks<0) debug fprintf(stderr,"ERROR: negative tasks\n"); UNLOCK(tasksMtx); //decrementa tasks

#define END_THREAD(fd,dir,buf,msg) endWorker(fd,dir,buf,msg,__LINE__,__FILE__) //chiama endWorker con line e file settata

#define WRITE_ERROR(fd,dir,block) if(writeMsg(fd,"KO error writing message \n", 27) == -1){perror("writeMsg");} END_THREAD(fd,dir,block,"ERROR: writeMsg"); //gestione errore in scrittura
#define READ_ERROR(fd,dir,block) if(writeMsg(fd,"KO error reading message \n", 27) == -1){perror("writeMsg");} END_THREAD(fd,dir,block,"ERROR: readMsg"); //gestione errore in lettura
#define CLOSE_SERVER(fd,dir,block) if(writeMsg(fd,"KO closing server \n", 20) == -1){perror("writeMsg");} END_THREAD(fd,dir,block,NULL); //chiudi il thread perchè sig_flag a 1

#define WRITE_MSG(fd,msg,dir) if(writeMsg(fd,msg,strlen(msg)+1) == -1){WRITE_ERROR(fd,dir,NULL);} //scrivi una stringa su fd

#define WRITE_BUF(fd,buf,len,dir) if(writeMsg(fd,buf,len) == -1){WRITE_ERROR(fd,dir,buf);} else{free(buf);} //scrivi un blocco di dati su fd

#define READ_MSG(block,fd,dir) if(waitFd(fd)==0){CLOSE_SERVER(fd,dir,NULL);} \
			       block = readMsgServer(fd);\
			       if(sig_flag){CLOSE_SERVER(fd,dir,block);} \
			       if(block == NULL){READ_ERROR(fd,dir,NULL);}	// leggi un blocco di dati controllando sig_flag	

static pthread_mutex_t statTotMtx[NUM_TOT]; //mutex per statTot
static pthread_mutex_t statOkMtx[NUM_OK]; //mutex per statOk
static pthread_mutex_t dataSizeMtx[2]; //mutex per dataSize
static pthread_mutex_t tasksMtx = PTHREAD_MUTEX_INITIALIZER; //mutex per tasks
static pthread_mutex_t bstMtx = PTHREAD_MUTEX_INITIALIZER; //mutex per il bst

static unsigned long long int statTot[NUM_TOT]; //numero operazioni effettuate per tipo
static unsigned long long int statOk[NUM_OK]; //numero operazioni effettuate con successo per tipo
static unsigned long long int dataSize[2]; //quantità byte memorizzati/recuperati dall'apertura del server

static unsigned long long int currentSize = 0; //dimensione totale ogetti presenti in data (settata da ftwOS)
static unsigned long long int numObj = 0; //numero ogetti presenti in data (settata da ftwOS)

static unsigned long long int tasks = 0; //thread attivi

int sig_flag = 0; //segnala chiusura server

static node* users; //bst utenti on-line

//******************************************************************
void* worker();

/*
DESCRIPTION: Crea un worker in modalità detached
PARAMETERS: fd -> file descriptor su cui lavorerà il worker
*/
void spawn_worker(int connfd) {
    pthread_attr_t thattr;
    pthread_t thid;
    
    if (pthread_attr_init(&thattr) != 0) {
	fprintf(stderr, "EEROR: pthread_attr_init at line %d of file %s\n",__LINE__,__FILE__);
	close(connfd);
	return;
    }
    if (pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0) {
	fprintf(stderr, "ERROR: pthread_attr_setdetachstate at line %d of file %s\n",__LINE__,__FILE__);
	pthread_attr_destroy(&thattr);
	close(connfd);
	return;
    }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	    if (pthread_create(&thid, &thattr, worker, (void*)connfd) != 0) {
		fprintf(stderr, "ERROR: pthread_create at line %d of file %s\n",__LINE__,__FILE__);
		pthread_attr_destroy(&thattr);
		close(connfd);
		return;
	    }
    #pragma GCC diagnostic pop

    if(pthread_attr_destroy(&thattr) != 0) {
	fprintf(stderr, "ERROR: pthread_attr_destroy at line %d of file %s\n",__LINE__,__FILE__);
    }
}

/*
DESCRIPTION: Funzione da passare a ftw per visita della cartella data: conta il numero di ogetti e la dimensione totale salvandole rispettivamente su numObj e currentSize
PARAMETERS: *vedere man 3 ftw*
RETURN VALUE: 0
*/
int ftwOS(const char* fpath, const struct stat *sb, int typeflag){

	if(typeflag == FTW_F){

		currentSize += sb->st_size;
		numObj++;

	}

	return 0;
}

/*
DESCRIPTION: Stampa statistiche sul server
*/
void debugStatistics(){

	unsigned long long int statTotVal[NUM_TOT];
	unsigned long long int statOkVal[NUM_OK];
	unsigned long long int dataSizeVal[2];
	unsigned long long int totVal = 0;
	unsigned long long int okVal = 0;
	currentSize = 0;
	numObj = 0;

	//accede a tutte le variabili condivise per averne una copia locale

	for(int i=0; i<NUM_TOT; i++){ 

		LOCK(statTotMtx[i]); 
		statTotVal[i] = statTot[i];
		UNLOCK(statTotMtx[i]);
		totVal += statTotVal[i];

		if(i<NUM_OK){

			LOCK(statOkMtx[i]);
			statOkVal[i] = statOk[i];
			UNLOCK(statOkMtx[i]);
			okVal += statOkVal[i];
		}

		if(i<2){
			
			LOCK(dataSizeMtx[i]);
			dataSizeVal[i] = dataSize[i];
			UNLOCK(dataSizeMtx[i]);
		}
	}	

	if(ftw("./data",ftwOS,MAX_FD) == -1){

		fprintf(stderr,"ERROR: ftw\n");

	}

	printf("********** SERVER STATES **********\n");
	printf("\nCLIENTS\n");
	printf("  Total: %llu \n",statOkVal[CON]);
	printf("  On line: %llu \n",statOkVal[CON] - statTotVal[DIS]);
	

	printf("\nOBJECTS\n");
	printf("  Total: %llu \n",statOkVal[STR]);
	printf("  Current: %llu \n",numObj);
	printf("  Current size: %llu \n",currentSize);
	

	printf("\nCONNECTIONS\n");
	printf("  Attempts: %llu \n",statTotVal[CON]);
	printf("  Success: %llu \n",statOkVal[CON]);
	

	printf("\nSTORES\n");
	printf("  Attempts: %llu \n",statTotVal[STR]);
	printf("  Success: %llu \n",statOkVal[STR]);
	

	printf("\nRETRIEVES\n");
	printf("  Attempts: %llu \n",statTotVal[RET]);
	printf("  Success: %llu \n",statOkVal[RET]);
	

	printf("\nDELETS\n");
	printf("  Attempts: %llu \n",statTotVal[DEL]);
	printf("  Success: %llu \n",statOkVal[DEL]);
	

	printf("\nDISCONNECTIONS\n");
	printf("  Total: %llu \n",statTotVal[DIS]);
	

	printf("\nOPERATIONS\n");
	printf("  Attempts: %llu \n",totVal);
	printf("  Success: %llu \n",okVal+statTotVal[DIS]);
	printf("\n  BYTE STORED: %llu \n", dataSizeVal[STORE]);
	printf("  BYTE RETRIEVED: %llu \n", dataSizeVal[RETRIEVE]);
	
	printf("\n***********************************\n");


}

//***********************************************************************************


/*
DESCRIPTION: Gestisce i segnali: se segnale != SIGUSR_1 setta sig_flag a 1, altrimenti chiama debugStatistics() e si rimiette in attesa
*/
void* sigHandler(void* useless) {

	pthread_detach(pthread_self());

	sigset_t set;        
	SIGFILLSET(set);
	int sig;
	ADD_TASK
	while(1){
		SIGWAIT(set, sig);

		if(sig == SIGUSR1) debugStatistics();
		else{
			debug fprintf(stderr,"\nTASKS: %llu\n",tasks);
			sig_flag = 1;
			REMOVE_TASK
			pthread_exit(NULL);
		}
	}

	return NULL;
}


/*
DESCRIPTION: Fa terminare un worker
PARAMETERS: fd -> file descriptor su cui lavorava il worker, dir -> indice i-node cartella relativa a utente servito dal worker, msg -> memora da liberare, errMsg -> messaggio di errore, line -> linea in cui avvenuta chiamata, file -> file in cui avvenuta chiamata
*/
void endWorker(int fd, int dir, void* msg, char* errMsg, int line, char* file){

	REMOVE_TASK
	if(errMsg != NULL) debug fprintf(stderr,"%s at line %d of file %s\n",errMsg,line,file);

	if(close(fd)==-1) fprintf(stderr,"ok\n");
	if(msg != NULL)free(msg);
	if(dir != -1){LOCK(bstMtx); deleteNode(users,dir); UNLOCK(bstMtx);}
	pthread_exit(NULL);

}

/*
DESCRIPTION: Worker che serve un client, intergisce sul file descriptor passato come argomento
PARAMETERS: arg -> se castato a int file descriptor su cui interagire con il client
*/
void* worker(void* arg){

	char *name, *msg, *token, *tmp;

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
		int fd = (int) arg;
	#pragma GCC diagnostic pop


	// attendi registrazione
	while(1){		

		READ_MSG(msg,fd,-1)

		token = strtok_r(msg," ",&tmp);
		if(token == NULL){WRITE_MSG(fd, "KO invalid header\n",-1); continue;}
		debug fprintf(stderr,"\n*** %s ",token); //------------------

		if(EQUALS(token,"REGISTER"))break;
		WRITE_MSG(fd, "KO not yet registered \n",-1);
	}
	INC_TOT(CON);
	ADD_TASK

	name = strtok_r(NULL," ",&tmp);
	if(name == NULL){
		WRITE_MSG(fd, "KO invalid header \n",-1);
		END_THREAD(fd,-1,msg,"ERROR: strtok_r");
	}

	debug fprintf(stderr,"%s\n",name); //------------------	

	//ottieni path cartella

	int dim = snprintf(NULL,0,"./data/%s",name);

	if(dim<0){
		WRITE_MSG(fd, "KO error during registration \n",-1);
		END_THREAD(fd,-1,msg,"ERROR: snprintf");
	}

	char* path = malloc(dim*sizeof(char)+1);
	if(path == NULL){
		WRITE_MSG(fd, "KO malloc error \n",-1);
		END_THREAD(fd,-1,msg,"ERROR: malloc");
	}

	if(snprintf(path,dim+1,"./data/%s",name)<0){
		WRITE_MSG(fd, "KO error during registration \n",-1);
		END_THREAD(fd,-1,path,"ERROR: snprintf");
	}

	free(msg);

	//crea cartella se non presente
	if(mkdir(path,DIR_MODE) == -1 && errno != EEXIST){ //se errore in mkdir diverso da cartella esistente
		WRITE_MSG(fd, "KO error during registration \n",-1);
		END_THREAD(fd,-1,path,"ERROR: mkdir");
	}

	int dirFd = open(path,O_RDONLY); // apri cartella
	
	struct stat file_stat;   
	if (fstat(dirFd, &file_stat) == -1) {  
	   	perror("fstat");
		WRITE_MSG(fd, "KO error during registration \n",-1);
		END_THREAD(fd,-1,path,"ERROR: fstat");
	} 
	
	close(dirFd);

	int dir = file_stat.st_ino; //indice i-node
	
	//verifica che user non ancora on-line
	LOCK(bstMtx);
	if(isIn(users,dir)){
		UNLOCK(bstMtx);
		WRITE_MSG(fd, "KO users alredy online \n",-1);
		END_THREAD(fd,-1,path,NULL);

	}
	else if(insertNode(users,dir) == NULL){
		UNLOCK(bstMtx);
		WRITE_MSG(fd, "KO error during registration \n",-1);
		END_THREAD(fd,dir,path,"ERROR: malloc");
	}
	UNLOCK(bstMtx);

	WRITE_MSG(fd,OK,dir);
	INC_OK(CON);

	while(!sig_flag){

		READ_MSG(msg,fd,dir)

		token = strtok_r(msg," ",&tmp);

		if(name == NULL){
			WRITE_MSG(fd, "KO invalid header \n",dir);
			PRINT_ERROR("ERROR: strtok_r");
			free(msg);
			continue;
		}

		debug fprintf(stderr,"*** %s ",token); //------------------
		
		
		if(EQUALS(token,"REGISTER")){
			INC_TOT(CON);
			WRITE_MSG(fd, "KO already registered \n",dir);
			free(msg);
			continue;
		}

		else if(EQUALS(token,"STORE")){
			INC_TOT(STR);
			char *nameData, *size;
			size_t len;
			
			nameData = strtok_r(NULL," ",&tmp);
			if(nameData == NULL){
				WRITE_MSG(fd, "KO invalid header \n",dir);
				PRINT_ERROR("ERROR: strtok_r");
				free(msg);
				continue;
			}
			debug fprintf(stderr,"%s ",nameData); //------------------
			size = strtok_r(NULL," ",&tmp);
			if(size == NULL){
				WRITE_MSG(fd, "KO invalid header \n",dir);
				PRINT_ERROR("ERROR: strtok_r");
				free(msg);
				continue;
			}
			len = strtol(size,NULL,0);


			debug fprintf(stderr,"%zu\n",len); //------------------

			if(len == 0L || len == LONG_MAX || len == LONG_MIN){
				WRITE_MSG(fd, "KO error in message header \n",dir);
				if(len != 0L) perror("strtol");
				PRINT_ERROR("ERROR: strtol");
				free(msg);
				continue;
			}
			
			void* block = malloc(len);
			if(waitFd(fd)==0){CLOSE_SERVER(fd,dir,NULL);} 
			if(readnServer(fd,block,sizeof(char)) == -1){READ_ERROR(fd,dir,NULL);} //leggi " "
			if(sig_flag){CLOSE_SERVER(fd,dir,block);}
			if(block == NULL){READ_ERROR(fd,dir,NULL);} 
			
			if(waitFd(fd)==0){CLOSE_SERVER(fd,dir,NULL);} 
			if(readnServer(fd, block, len) == -1){READ_ERROR(fd,dir,NULL);}
			if(sig_flag){CLOSE_SERVER(fd,dir,block);}
			if(block == NULL){READ_ERROR(fd,dir,NULL);} 

			if(store(path,nameData,len,block)){
				free(block);
				ADD_DATA(STORE,len)
				WRITE_MSG(fd,OK,dir);
				INC_OK(STR);
			}
			else{
				free(block);
				WRITE_MSG(fd, "KO error while storing data \n",dir);
				PRINT_ERROR("ERROR: store");
				free(msg);
				continue;	
			}
			
		}

		else if(EQUALS(token,"RETRIEVE")){
			INC_TOT(RET);
			
			char* nameData = strtok_r(NULL," ",&tmp);
			if(nameData == NULL){
				WRITE_MSG(fd, "KO invalid header \n",dir);
				PRINT_ERROR("ERROR: strtok_r");
				free(msg);
				continue;
			}
			debug fprintf(stderr,"%s ",nameData); //------------------

			size_t len;
			void* data = retrieve(path,nameData,&len);

			if(data == NULL){
				debug fprintf(stderr,"\n");
				WRITE_MSG(fd, "KO error during data recovery \n",dir);
				PRINT_ERROR("ERROR: retrieve");
				free(msg);
				continue;
			}
			
			int dim = snprintf(NULL,0,"DATA %zu \n ",len);
			if(dim<0){
				debug fprintf(stderr,"\n");
				WRITE_MSG(fd, "KO error during data recovery \n",dir);
				PRINT_ERROR("ERROR: snprintf");
				free(msg);
				continue;
			}

			void* ans = malloc(dim*sizeof(char)+1);
			if(ans == NULL){
				debug fprintf(stderr,"\n");
				WRITE_MSG(fd, "KO malloc error \n",dir);
				PRINT_ERROR("ERROR: malloc");
				free(msg);
				continue;
			}
	
			if(snprintf(ans,dim+1,"DATA %zu \n ",len)<0){
				debug fprintf(stderr,"\n");
				WRITE_MSG(fd, "KO error during data recovery \n",dir);
				PRINT_ERROR("ERROR: snprintf");
				free(msg);
				continue;
			}
	
			debug fprintf(stderr,"%zu\n",len); //------------------

			WRITE_BUF(fd,ans, dim*sizeof(char)+1,dir);
			WRITE_BUF(fd, data, len, dir);

			ADD_DATA(RETRIEVE,len)
			
			INC_OK(RET);
		}

		else if(EQUALS(token,"DELETE")){
			INC_TOT(DEL);
			char* nameData = strtok_r(NULL," ",&tmp);
			if(nameData == NULL){
				WRITE_MSG(fd, "KO invalid header \n",dir);
				PRINT_ERROR("ERROR: strtok_r");
				free(msg);
				continue;
			}
			debug fprintf(stderr,"%s\n",nameData); //------------------

			if(!deleteFile(path,nameData)){

				WRITE_MSG(fd, "KO error during data deletion \n",dir);
				PRINT_ERROR("ERROR: delete");
				free(msg);
				continue;
			}
			
			WRITE_MSG(fd,OK,dir);
			INC_OK(DEL);
		}

		else if(EQUALS(token,"LEAVE")){
			debug fprintf(stderr,"\n");
			INC_TOT(DIS);
			WRITE_MSG(fd,OK,dir);		
			break;
		}

		else{
			WRITE_MSG(fd,"KO unrecognized command \n",dir);
		}
		free(msg);

	}
	
	free(path);
	END_THREAD(fd,dir,msg,NULL);
	return NULL;
}

int main(){

	int fd_c, client;
	pthread_attr_t tattr;
	pthread_t sig_handler_tid;

	users = new_node(-42); //aggiunge un nodo inutile per evitare incosistenze con le delete dei worker

	if(users == NULL){
		fprintf(stderr, "ERROR: malloc at line %d of file %s\n",__LINE__,__FILE__);
		exit(EXIT_FAILURE);
	}

	//blocca tutti i segnali: i thread la ereditano

	sigset_t set;
	SIGFILLSET(set);
	SIGMASK(SIG_BLOCK, set);	

	// crea sigHandler in modalità detached

	if (pthread_attr_init(&tattr) != 0) {
		fprintf(stderr, "ERROR: pthread_attr_init at line %d of file %s\n",__LINE__,__FILE__);
		exit(EXIT_FAILURE);
	}
	if (pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED) != 0) {
		fprintf(stderr, "ERROR: pthread_attr_setdetachstate at line %d of file %s\n",__LINE__,__FILE__);
		pthread_attr_destroy(&tattr);
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&sig_handler_tid, &tattr, sigHandler, NULL) != 0) {
		fprintf(stderr, "ERROR: pthread_create at line %d of file %s\n",__LINE__,__FILE__);
		pthread_attr_destroy(&tattr);
		exit(EXIT_FAILURE);
	}


	if(pthread_attr_destroy(&tattr) != 0) {
		fprintf(stderr, "ERROR: pthread_attr_destroy at line %d of file %s\n",__LINE__,__FILE__);
		exit(EXIT_FAILURE);
	}
	
	if(mkdir("./data",DIR_MODE) == -1 && errno != EEXIST){
		perror("mkdir");
		exit(errno);
	}

	debug printf("*** SERVER ***\n");

	//inizializza variabili condivise per stato server

	for(int i=0; i<NUM_TOT; i++){

		statTot[i]=0; 
		INIT((statTotMtx[i]));
		
		if(i<NUM_OK){		
			statOk[i]=0; 
			INIT((statOkMtx[i%NUM_OK]));
		}
		
		if(i<2){
			dataSize[i]=0;
			INIT((dataSizeMtx[i%2]));
		}
	}

	client = SOCKET(AF_UNIX, SOCK_STREAM,0);
	
	struct sockaddr_un sa;      
	strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);
	sa.sun_family=AF_UNIX;

	BIND(client,(struct sockaddr *)&sa,sizeof(sa));
	LISTEN(client,SOMAXCONN);

	while(1){
		if(waitFd(client) == 0)break; //sig_flag a 1
		fd_c=ACCEPT(client,NULL,0);
		
		spawn_worker(fd_c);
		
	}
	
	while(tasks>0){debug fprintf(stderr,"\nTASKS: %llu\n",tasks); debug sleep(1);} //attende la chiusura dei workers e sigHandler

	if(unlink(SOCKNAME) == -1) perror("unlink sock");
	debug fprintf(stderr,"\nTASKS: %llu\n",tasks);
	free(users);
	debug fprintf(stderr,"\nBYE\n"); 
	debug fflush(stderr);

	debug printf("*** END SERVER ***\n");
	
	return 0;
}
