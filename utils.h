#ifndef UTILS_H
#define UTILS_H

#define DEBUG 0

#define _POSIX_C_SOURCE  200112L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <sys/wait.h>
#include <sys/select.h>
#include <limits.h>
#include <poll.h>
#include <ftw.h>

#include <conn.h>

//ERROR MANAGMENT
#define CHECK(bool,string) if(!bool){perror(string); exit(errno);}
#define CHECK_EQ(X,val,string) if((X)==val){perror(string); exit(errno);}
#define CHECK_NEQ(X,val,string) if((X)!=val){perror(string); exit(errno);}
#define CHECK_NULL(X,string) CHECK_EQ(X,NULL,string)
#define CHECK_M1(X,string) CHECK_EQ(X,-1,string)

void CHECK_THREADCALL(int val, const char* string){

	if(val!=0){
		perror(string); 
		errno=val; 

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
			pthread_exit((void*)val);
		#pragma GCC diagnostic pop
	}

}

int SYSCALL(int val, const char* string){

	CHECK_M1(val,string)
	return val;

}


//FILES && DIRECTORIES
#define OPEN(path,flag) SYSCALL(open(path,flag),"open")
#define CREATE(path,flag,perm) SYSCALL(open(path,flag,perm),"open")
#define READ(fd,buff,n) SYSCALL(read(fd,buff,n),"read")
#define WRITE(fd,buff,n) SYSCALL(write(fd,buff,n),"write")
#define CLOSE(fd) SYSCALL(close(fd),"close")
#define UNLINK(path) SYSCALL(unlink(path),"unlink")

//PROCCESS
#define FORK SYSCALL(fork(),"fork")
#define WAITPID(pid,status,opt) SYSCALL(waitpid(pid,status,opt),"waitpid")

//THREADS
#define THREAD(tid,attr,fun,arg) CHECK_THREADCALL(pthread_create(&tid, attr, &fun, arg),"create thread")
#define THREAD_JOIN(tid,status) CHECK_THREADCALL(pthread_join(tid,status),"join thread")
#define LOCK(mtx) CHECK_THREADCALL(pthread_mutex_lock(&mtx),"lock mutex")
#define UNLOCK(mtx) CHECK_THREADCALL(pthread_mutex_unlock(&mtx),"unlock mutex")
#define WAIT(cond,mtx) CHECK_THREADCALL(pthread_cond_wait(&cond, &mtx),"wait cond var")
#define SIGNAL(cond) CHECK_THREADCALL(pthread_cond_signal(&cond),"signal cond var")
#define DETACH(tid) CHECK_THREADCALL(pthread_detach(tid),"detach")
#define INIT(mtx) CHECK_THREADCALL(pthread_mutex_init(&mtx,NULL),"init")

//PIPES
#define PIPE(pfd) SYSCALL(pipe(pfd),"create pipe")
#define DUP(fd) SYSCALL(dup(fd),"dup")
#define DUP2(fd,fd2) SYSCALL(dup2(fd,fd2),"dup2")

//SOCKETS
#define SOCKET(domain,type,protocol) SYSCALL(socket(domain,type,protocol),"socket")
#define BIND(fd,addr,len) SYSCALL(bind(fd,addr,len),"bind")
#define LISTEN(fd,log) SYSCALL(listen(fd,log),"listen")
#define ACCEPT(fd,addr,len) SYSCALL(accept(fd,addr,len),"accept")
#define CONNECT(fd,addr,len) SYSCALL(connect(fd,addr,len),"connect")
#define SELECT(n,rd,wr,err,timeout) SYSCALL(select(n,rd,wr,err,timeout),"select")

#define SIGACTION(sig,sa,old) if(sigaction(sig,&sa,old) == -1){ fprintf(stderr, "ERROR: sigaction\n"); exit(EXIT_FAILURE);}
#define SIGWAIT(set,sig) if(sigwait(&set,&sig) != -0){ fprintf(stderr, "ERROR: sigwait\n"); exit(EXIT_FAILURE); }
#define SIGFILLSET(set) if(sigfillset(&set) == -1){ fprintf(stderr, "ERROR: sigfillset\n"); exit(EXIT_FAILURE); }
#define SIGEMPTYSET(set) if(sigemptyset(&set) == -1){ fprintf(stderr, "ERROR: sigemptyset\n"); exit(EXIT_FAILURE); }
#define SIGDELSET(set,sig) if(sigdelset(&set,sig) == -1){ fprintf(stderr, "ERROR: sigdelset\n"); exit(EXIT_FAILURE); }
#define SIGADDSET(set,sig) if(sigaddset(&set,sig) == -1){ fprintf(stderr, "ERROR: sigaddset\n"); exit(EXIT_FAILURE); }
#define SIGMASK(mode,set) if (pthread_sigmask(mode, &set, NULL) != 0) { fprintf(stderr, "ERROR: pthread_sigmask\n"); exit(EXIT_FAILURE); }


void* MALLOC(size_t size){

	void* tmp;
	tmp = malloc(size);
	CHECK_NULL(tmp,"malloc")
	return tmp;

}

long FPATHCONF(int fd, int name){
	errno = 0;
	long int v;
	if((v = fpathconf(fd,name))==-1){
		if (errno != 0) { perror("fpathconf"); exit(errno);}
	}
	return v;
}

long PATHCONF(char* path, int name){
	errno = 0;
	long int v;
	if((v = pathconf(path,name))==-1){
		if (errno != 0) { perror("pathconf"); exit(errno);}
	}
	return v;
}


//***************************************************************

void print(char* str){

	if(writen(1,str,strlen(str)) == -1){

		perror("print");
		exit(errno);
	
	}

}

//***************************************************************

#define UNIX_PATH_MAX 108
#define SOCKNAME "./objstore.sock"

#define END_MSG "LEAVE \n"
#define OK "OK \n"

#define EQUALS(a,b) strncmp(a,b,strlen(b)) == 0

#define debug if(DEBUG)

#define PRINT_ERROR(errMsg) debug fprintf(stderr,"%s at line %d of file %s\n",errMsg,__LINE__,__FILE__);

#endif

