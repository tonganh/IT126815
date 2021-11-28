CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: clear clean client server

client: client.c clientHelper.c checkinput.c caroRanking.c
	${CC} client.c clientHelper.c  checkinput.c caroRanking.c -o client

server: server.c serverHelper.c caroai.c checkinput.c linklist.c caroRanking.c
	${CC} server.c  serverHelper.c caroai.c checkinput.c linklist.c caroRanking.c -o server
clean:
	rm -f *.o *~
clear:
	clear