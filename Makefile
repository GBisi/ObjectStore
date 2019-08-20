CC		=  gcc
AR              =  ar
CFLAGS	        += -std=c99 -Wall
ARFLAGS         =  rvs
INCLUDES	= -I .
LDFLAGS 	= -L .
OPTFLAGS	= -O3 
LIBS            = -lpthread -lObjStore

TARGETS		= server       \
		  client       

.PHONY: all clean test Server Client Client1 Client2 Client3 valServer valClient
.SUFFIXES: .c .h

#----------------------------------------------------------------------------

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

#----------------------------------------------------------------------------

all: $(TARGETS)

server: server.c libBST.a serverUtilities.h utils.h conn.h connServer.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< BST.o $(LDFLAGS) -lpthread -lBST

client: client.c libObjStore.a
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) -lObjStore

libObjStore.a: ObjStore.o ObjStore.h utils.h conn.h
	$(AR) $(ARFLAGS) $@ $<
	rm -f ObjStore.o 

libBST.a: BST.o BST.h
	$(AR) $(ARFLAGS) $@ $<

ObjStore.o: ObjStore.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

BST.o:	BST.c BST.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<


#----------------------------------------------------------------------------

test: client server test.sh
	chmod +x ./test.sh
	echo "LAUNCHING SERVER..."
	rm -f objstore.sock
	rm -rf data
	./server &
	echo "SERVER ON LINE"
	./test.sh

Server:  server
	rm -f objstore.sock
	rm -rf data
	./server 

Client: client
	./client TEST 0

Client1: client
	./client TEST 1

Client2: client
	./client TEST 2

Client3: client
	./client TEST 3

valServer: server
	rm -f objstore.sock
	rm -rf data
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./server

valClient: client
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./client TEST 0

clean		: 
	rm -f $(TARGETS) libObjStore.a libBST.a objstore.sock BST.o testout.log
	rm -rf data
