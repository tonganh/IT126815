CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: resolver
resolver: hieu.c
	${CC}  hieu.c -o resolver
clean:
	rm -rf *.o *.~ | rm -rf resolver
