#ifndef CONN_H
#define CONN_H


/*
DESCRIPTION: Legge dal file descriptor fino ad un massimo di size byte senza interrompersi per segnali
PARAMETERS: fd -> file descriptor, buf -> memoria su cui salavare i byte, size -> massima quantità di byte leggibile
RETURN VALUE: quantità byte letti, -1 in caso di errore
*/
static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=read((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;   // gestione chiusura socket
        left    -= r;
	bufptr  += r;
    }
    return size;
}

/*
DESCRIPTION: Scrive dal file descriptor size byte senza interrompersi per segnali
PARAMETERS: fd -> file descriptor, buf -> byte da scrivere, size -> quantità byte scrivibili
RETURN VALUE: 1 se scrittura effettuata, -1 in caso di errore, 0 se size è 0 e nessun errore è avvenuto
*/
static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;  
        left    -= r;
	bufptr  += r;
    }
    return 1;
}

/*
DESCRIPTION: Legge dal file descriptor un carattere alla volta fino alla lettura di '\n'
PARAMETERS: fd -> file descriptor
RETURN VALUE: la stringa letta, NULL in caso di errore
*/
char* readMsg(int fd){
	
	char chr;
	char* tmp = NULL;
	char* buff = NULL;
	int i=0;
	do{
		if(readn(fd, &chr, sizeof(char)) == -1){perror("readn"); return NULL;}
		i++;
		tmp = (char*)realloc(buff,i*sizeof(char)+1);
		if (tmp == NULL){perror("realloc"); return NULL;}
		else buff = tmp;
		buff[i-1] = chr;
	}while(chr != '\n');

	if(readn(fd, &chr, sizeof(char)) == -1){perror("readn"); return NULL;}

	i++;
	tmp = (char*)realloc(buff,i*sizeof(char)+1);
	if (tmp == NULL){perror("realloc"); return NULL;}
	else buff = tmp;
	buff[i-1] = '\0';

	return buff;
}

/*
DESCRIPTION: Scrive dal file descriptor size byte senza interrompersi per segnali
PARAMETERS: fd -> file descriptor, buf -> byte da scrivere, size -> quantità byte scrivibili
RETURN VALUE: 0 se scrittura effettuata, -1 in caso di errore
*/
int writeMsg(int fd, void* buf, size_t size){

	if(writen(fd, buf, size) == -1){perror("writeMsg"); return -1;}
	return 0;

}

#endif
