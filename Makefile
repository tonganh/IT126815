CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: week1
week1: code.o stack.o
	${CC} code.o stack.o -o week1
code.o: code.c
	${CC} ${CFLAGS} code.c
stack.o : stack.c
	${CC} ${CFLAGS} stack.c

clean:
	rm -rf *.o *.~