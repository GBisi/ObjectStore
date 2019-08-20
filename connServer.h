#ifndef CONN_SERVER_H
#define CONN_SERVER_H

#define TIME 50 //milliseconds

extern int sig_flag;

/*
DESCRIPTION: Si mette in attesa di un qualche evento sul file descriptor, se sig_flag == 1 termina
PARAMETERS: fd -> file descriptor
RETURN VALUE: 0 se sig_flag == 1, -1 in caso di errore, 1 altrimenti
*/
int waitFd(int fd){

	struct pollfd pFd[1];
	pFd[0] = (struct pollfd){fd, POLLIN, 0};

	if(sig_flag){return 0;}
	while(1){
		int val = poll(pFd,1,TIME);
		if(sig_flag){return 0;}
		else if(val == -1){perror("poll"); return -1;}
		else if (val > 0){return 1;}
	}

}

/*
DESCRIPTION: Legge dal file descriptor fino ad un massimo di size byte senza interrompersi per segnali, se sig_flag == 1 termina
PARAMETERS: fd -> file descriptor, buf -> memoria su cui salavare i byte, size -> massima quantità di byte leggibile
RETURN VALUE: quantità byte letti, -1 in caso di errore, -2 se sig_flag == 1
*/
static inline int readnServer(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if(waitFd(fd)==0) return -2;
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
DESCRIPTION: Legge dal file descriptor un carattere alla volta fino alla lettura di '\n', se sig_flag == 1 termina
PARAMETERS: fd -> file descriptor
RETURN VALUE: la stringa letta, NULL in caso di errore o se sig_flag == 1
*/
char* readMsgServer(int fd){
	
	char chr;
	char* tmp = NULL;
	char* buff = NULL;
	int r;
	int i=0;
	do{
		if((r=readnServer(fd, &chr, sizeof(char))) == -1){perror("readn"); return NULL;}
		if(r==-2) return NULL;
		i++;
		tmp = (char*)realloc(buff,i*sizeof(char)+1);
		if (tmp == NULL){perror("realloc"); return NULL;}
		else buff = tmp;
		buff[i-1] = chr;
	}while(chr != '\n');

	if((r=readnServer(fd, &chr, sizeof(char))) == -1){perror("readn"); return NULL;}
	if(r==-2) return NULL;
	i++;
	tmp = (char*)realloc(buff,i*sizeof(char)+1);
	if (tmp == NULL){perror("realloc"); return NULL;}
	else buff = tmp;
	buff[i-1] = '\0';

	return buff;
}

#endif
