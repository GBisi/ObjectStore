#include <utils.h>

#define CHECK_VAL(f,v) if(f==v) return 0;

#define WRITE_MSG(fd,str,len) CHECK_VAL(writeMsg(fd,str,len),-1);
#define READ_MSG(str,fd) str = readMsg(fd); CHECK_VAL(str,NULL);

static int server = -1; //file descriptor del server

int os_connect(char *name){

	if(server != -1) return 0; //connessione già esistente

	server = socket(AF_UNIX, SOCK_STREAM,0);
	CHECK_VAL(server,-1)
	
	struct sockaddr_un sa;      
	strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);
	sa.sun_family=AF_UNIX;

	CHECK_VAL(connect(server,(struct sockaddr*)&sa,sizeof(sa)),-1)

	int dim = snprintf(NULL,0,"REGISTER %s \n",name);
	if(dim<0) return 0;

	char* request = malloc(dim*sizeof(char)+1);
	CHECK_VAL(request,NULL)

	if(snprintf(request,dim+1,"REGISTER %s \n",name)<0){free(request); return 0;}

	WRITE_MSG(server,request, dim+1)

	free(request);

	char* ans=NULL; READ_MSG(ans,server)

	if(EQUALS(ans,OK)){free(ans); return 1;}
	debug fprintf(stderr,"%s\n",ans);
	free(ans);

	return 0;

}
int os_store(char *name, void *block, size_t len){

	if(server == -1) return 0; //connessione non effettuata

	int dim = snprintf(NULL,0,"STORE %s %zu \n ",name,len);
	if(dim<0) return 0;

	char* request = malloc(dim*sizeof(char)+1);
	CHECK_VAL(request,NULL)
	
	if(snprintf(request,dim+1,"STORE %s %zu \n ",name,len)<0){free(request); return 0;}

	WRITE_MSG(server,request,dim+1)
	WRITE_MSG(server, block, len);
	
	free(request);

	char* ans=NULL; READ_MSG(ans,server);

	if(EQUALS(ans,OK)){free(ans); return 1;}
	debug fprintf(stderr,"%s\n",ans);
	free(ans);

	return 0;

}
void* os_retrieve(char *name){

	if(server == -1) return NULL; //connessione non effettuata

	int dim = snprintf(NULL,0,"RETRIEVE %s \n",name);
	if(dim<0) return 0;

	char* request = malloc(dim*sizeof(char)+1);
	if(request == NULL) return NULL;
	
	if(snprintf(request,dim+1,"RETRIEVE %s \n",name)<0){free(request); return NULL;}
	
	WRITE_MSG(server,request, dim+1)

	free(request);

	char* ans=NULL; READ_MSG(ans,server)

	if(EQUALS(ans,"DATA")){
		char* tmp, *token;
		if(strtok_r(ans," ",&tmp) == NULL){free(ans); return NULL;}
		token = strtok_r(NULL," ",&tmp);
		if(token == NULL){free(ans); return NULL;}
		size_t len = strtol(token,NULL,0);
		if(len == 0L || len == LONG_MAX || len == LONG_MIN){free(ans); return NULL;}

		void* block = malloc(len); 
		if(readn(server, block, sizeof(char)) == -1){free(ans); return NULL;} //legge lo spazio
		if(readn(server, block, len) == -1){free(ans); return NULL;}
		free(ans); 

		return block;
	}

	debug fprintf(stderr,"%s\n",ans);
	free(ans);

	return NULL;

}
int os_delete(char *name){

	if(server == -1) return 0; //connessione non effettuata

	int dim = snprintf(NULL,0,"DELETE %s \n",name);
	if(dim<0) return 0;

	char* request = malloc(dim*sizeof(char)+1);
	CHECK_VAL(request,NULL)
	
	if(snprintf(request,dim+1,"DELETE %s \n",name)<0){free(request); return 0;}

	WRITE_MSG(server,request, dim+1)

	free(request);

	char* ans=NULL; READ_MSG(ans,server)

	if(EQUALS(ans,OK)){free(ans); return 1;}
	debug fprintf(stderr,"%s\n",ans);
	free(ans);

	return 0;

}
int os_disconnect(){

	if(server == -1) return 0; //connessione non effettuata

	char* close = END_MSG;
	WRITE_MSG(server,close,strlen(close)+1)

	char* ans=NULL; READ_MSG(ans,server)
	
	if(EQUALS(ans,OK)){free(ans); server = -1; return 1;}
	debug fprintf(stderr,"%s\n",ans);
	free(ans);

	return 0;

}
