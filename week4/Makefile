CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: clean server client


client: stack.o client.o
	${CC} stack.o client.o -o client

server: stack.o server.o
	${CC} stack.o server.o -o server

server.o: server.c
	${CC} ${CFLAGS} server.c
	
client.o : client.c
	${CC} ${CFLAGS} client.c

stack.o : stack.c
	${CC} ${CFLAGS} stack.c

clean:
	rm -rf *.o *.~ | clear