CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: client server

client: client.c clientHelper.c tic-tac-toe.c checkinput.c tictactoeRanking.c caroRanking.c
	${CC} client.c clientHelper.c tic-tac-toe.c checkinput.c tictactoeRanking.c caroRanking.c -o client

server: server.c serverHelper.c caroai.c tic-tac-toe.c checkinput.c linklist.c tictactoeRanking.c caroRanking.c
	${CC} server.c serverHelper.c caroai.c tic-tac-toe.c checkinput.c linklist.c tictactoeRanking.c caroRanking.c -o server

clean:
	rm -f *.o *~
