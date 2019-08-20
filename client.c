#include <ObjStore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define EQUALS(a,b) strncmp(a,b,strlen(b)) == 0
#define STRING "0123456789"

int strOp = 0; //numero store effettuate
int retOp = 0; //numero retrieve effettuate
int delOp = 0; //numero delete effettuate
int strFail = 0; //numero store fallite
int retFail = 0; //numero retrieve fallite
int delFail = 0; //numero delete fallite
int strData = 0; //byte memorizzati con successo
int retData =  0; //byte recuperati con successo
int con = 1; //esito connect
int dis = 0; //esito disconnect


/*
DESCRIPTION: Richiede al server di memorizzare un blocco di dati contenente la stringa STRING ripetuta size/10 volte
PARAMETERS: name -> memoria per nome, size -> dimensione dati (in byte)
*/
void store(char* name, size_t size){

	static int i=0;

	char* data = (char*)malloc((size+1)*sizeof(char));
	if(data == NULL){perror("malloc"); return;}

	strcpy(data,STRING);
	for(int j=1; j<size/10; j++){					
		strcat(data,STRING);
	}

	if(snprintf(name,6*sizeof(char),"obj%d",i++) < 0){fprintf(stderr,"ERROR: snprintf\n"); return;}

	strOp++;
	if(!os_store(name,data, size*sizeof(char))){strFail++;}
	else strData+=size*sizeof(char);
	
	free(data);

}

/*
DESCRIPTION: Effettua 20 store con dimensioni crescenti
*/
void stores(){

	char *name;
	size_t size = 100;

	name = malloc(6*sizeof(char));
	if(name == NULL){perror("malloc"); return;}	

	store(name,size);

	for(int i=1; i<19; i++){
		size += 5300;
		store(name,size);
	}
	

	size = 100000;
	store(name,size);

	free(name);

}

/*
DESCRIPTION: Richiede al server di recuperare un blocco di dati che dovrebbe contenere la stringa STRING ripetuta size/10 volte
PARAMETERS: name -> memoria per nome, size -> dimensione dati (in byte)
*/
void retrieve(char* name, size_t size){

	char *controlData, *data;
	static int i=0;

	controlData = (char*)malloc((size+1)*sizeof(char));
	if(controlData == NULL){perror("malloc"); return;}

	strcpy(controlData,STRING);
	for(int j=1; j<10; j++){					
		controlData = strcat(controlData,STRING);
	}
	
	if(snprintf(name,6*sizeof(char),"obj%d",i++) < 0){fprintf(stderr,"ERROR: snprintf\n"); return;} 

	retOp++;
	data = os_retrieve(name);

	if(data == NULL){retFail++; free(controlData); return;}

	if(!EQUALS(data,controlData)){retFail++;}
	else retData+=size*sizeof(char);
	free(data);
	free(controlData);

}

/*
DESCRIPTION: Effettua 20 retrieve con dimensioni crescenti
*/
void retrieves(){

	char* name;
	int size = 100;

	name = malloc(6*sizeof(char));
	if(name == NULL){perror("malloc"); return;}	

	retrieve(name,size);

	for(int i=1; i<19; i++){

		size += 5300;
		retrieve(name,size);

	}
	
	size = 100000;
	retrieve(name,size);

	free(name);

}

/*
DESCRIPTION: Effettua 20 cancellazioni con nomi obj$Indice 
*/
void deletes(){

	char *name;

	name = malloc(6*sizeof(char));
	if(name == NULL){perror("malloc"); return;}	

	for(int i=0; i<20; i++){

		if(snprintf(name,6*sizeof(char),"obj%d",i) < 0){fprintf(stderr,"ERROR: snprintf\n"); return;}
		delOp++;
		if(!os_delete(name)){delFail++;}

	}

	free(name);

}
/*
DESCRIPTION: Stampa il report del test
*/
void debug(char* arg){

	if(!con){
		printf("REPORT 1 1\n");
		printf("CONNECT 0\n");
		return;
	}

	int totOp = strOp+delOp+retOp+2;
	int totFail = strFail+delFail+retFail;
	if(!dis)totFail++;

	printf("REPORT %d %d\n",totOp, totFail);
	printf("CONNECT %d\n",con);
	if(EQUALS(arg,"1") || EQUALS(arg,"0"))printf("STORE %d %d %d\n",strOp, strFail, strData);
	if(EQUALS(arg,"2") || EQUALS(arg,"0"))printf("RETRIEVE %d %d %d\n",retOp, retFail, retData);
	if(EQUALS(arg,"3") || EQUALS(arg,"0"))printf("DELETE %d %d\n",delOp, delFail);
	printf("LEAVE %d\n",dis);
}

int main(int argc, char* argv[]){
	
	if(os_connect(argv[1])){		

		if(EQUALS(argv[2],"1")){stores();}
		else if(EQUALS(argv[2],"2")){retrieves();}
		else if(EQUALS(argv[2],"3")){deletes();}
		else if(EQUALS(argv[2],"0")){stores(); retrieves(); deletes();}
		else{printf("ERROR: %s do not recognized\n",argv[2]);}
		
		dis = os_disconnect();
	}
	else con = 0;
	
	debug(argv[2]);

	return 0;
}

