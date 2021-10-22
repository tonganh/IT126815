CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: client server

server: stack.o server.o
	${CC} stack.o server.o -o server

server.o: server.c
	${CC} ${CFLAGS} server.c
	
client : client.c
	${CC} client.c -o client

stack.o : stack.c
	${CC} ${CFLAGS} stack.c

clean:
	rm -rf *.o *.~ | clear