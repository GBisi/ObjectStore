#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <dirent.h>
#include <utils.h>
#include <connServer.h>

#define FILE_MODE 0777

/*
DESCRIPTION: Crea un file su cui memorizza un blocco di dati
PARAMETERS: path -> percorso della directory in cui creare il file, name -> nome file, len -> dimensione dati (in byte), block -> dati da memorizzare
RETURN VALUE: 1 se memorizzazione effettuata, 0 altrimenti
*/
int store(char* path, char* name, size_t len, void* block){

	int dim = strlen(path)+strlen(name)+1;
	char* filePath = malloc(sizeof(char)*(dim+1));
	if(filePath == NULL){PRINT_ERROR("ERROR: malloc"); return 0;}
	if(snprintf(filePath,sizeof(char)*(dim+1),"%s/%s",path,name) < 0){free(filePath); PRINT_ERROR("ERROR: snprintf"); return 0;}

	int fd = open(filePath,O_WRONLY|O_CREAT|O_EXCL,FILE_MODE); // fallisce se file già presente
	free(filePath);

	if(fd == -1){
		if(errno != EEXIST)perror("open");
		PRINT_ERROR("ERROR: open");
		return 0;
	}

	if(writen(fd,block,len) == -1){
		close(fd);
		perror("writen");
		PRINT_ERROR("ERROR: write");
		return 0;	
	}
	close(fd);
	return 1;
}

/*
DESCRIPTION: Recupera un blocco di dati memorizzato su un file
PARAMETERS: path -> percorso della directory dove si trova il file, name -> nome file, len -> puntatore in cui salvare la dimensione del blocco (in byte)
RETURN VALUE: il blocco di dati, NULL in caso di errore
*/
void* retrieve(char* path, char* name, size_t* len){

	int dim = strlen(path)+strlen(name)+1;
	char* filePath = malloc(sizeof(char)*(dim+1));
	if(filePath == NULL){PRINT_ERROR("ERROR: malloc"); return NULL;}
	if(snprintf(filePath,sizeof(char)*(dim+1),"%s/%s",path,name) < 0){free(filePath); PRINT_ERROR("ERROR: snprintf"); return NULL;}	

	int fd = open(filePath,O_RDONLY);
	free(filePath);
	if(fd == -1){
		perror("open");
		PRINT_ERROR("ERROR: open");
		return NULL;
	}

	int sz = lseek(fd,0,SEEK_END);
	if(sz == -1){perror("lseek"); PRINT_ERROR("ERROR: lseek"); return NULL;}
	*len = (size_t)sz;
	if(lseek(fd,0,SEEK_SET) == -1){perror("lseek"); PRINT_ERROR("ERROR: lseek"); return NULL;}

	void* data = malloc(sz);
	if(data == NULL){PRINT_ERROR("ERROR: malloc"); return NULL;}

	if(readn(fd,data,sz) == -1){perror("read"); PRINT_ERROR("ERROR: read"); return NULL;}

	close(fd);
	
	return data;

}

/*
DESCRIPTION: Elimina file in cui è memorizzato un blocco di dati
PARAMETERS: path -> percorso della directory in cui è presente il file, name -> nome file
RETURN VALUE: 1 se cancellazione effettuata, 0 altrimenti
*/
int deleteFile(char* path, char* name){

	int dim = strlen(path)+strlen(name)+1;
	char* filePath = malloc(sizeof(char)*(dim+1));
	if(filePath == NULL){PRINT_ERROR("ERROR: malloc"); return 0;}
	if(snprintf(filePath,sizeof(char)*(dim+1),"%s/%s",path,name) < 0){free(filePath); PRINT_ERROR("ERROR: snprintf"); return 0;}	

	if(unlink(filePath) == -1){
		perror("unlink");
		PRINT_ERROR("ERROR: unlink");
		free(filePath);
		return 0;
	}
	free(filePath);
	return 1;
}

#endif
