CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: resolver
resolver: nhat.c
	${CC}  nhat.c -o resolver
clean:
	rm -rf *.o *.~ | rm -rf resolver
