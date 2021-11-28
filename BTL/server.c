/*
Members:
-----Tong Ngoc Anh-----
-----Nguyen Thi Ngoc Diem-----
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
Library of socket
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "linklist.h"
#include "checkinput.h"
#include "serverHelper.h"
#include "caroRanking.h"
#include "caroai.h"

#define BUFF_SIZE 1024

// ANHTNGAME
#define SIGNAL_CHECKLOGIN "SIGNAL_CHECKLOGIN"
#define SIGNAL_CREATEUSER "SIGNAL_CREATEUSER"
#define SIGNAL_OK "SIGNAL_OK"
#define SIGNAL_ERROR "SIGNAL_ERROR"
#define SIGNAL_CLOSE "SIGNAL_CLOSE"

// caro game
#define SIGNAL_CARO_NEWGAME "SIGNAL_CARO_NEWGAME"
#define SIGNAL_CARO_ABORTGAME "SIGNAL_CARO_ABORTGAME"
#define SIGNAL_CARO_TURN "SIGNAL_CARO_TURN"
#define SIGNAL_CARO_WIN "SIGNAL_CARO_WIN"
#define SIGNAL_CARO_LOST "SIGNAL_CARO_LOST"
#define SIGNAL_CARO_VIEWLOG "SIGNAL_CARO_VIEWLOG"
#define SIGNAL_LOGLINE "SIGNAL_LOGLINE"

// caro ranking
#define SIGNAL_CARO_RANKING "SIGNAL_CARO_RANKING"

// tictactoe game
#define SIGNAL_TICTACTOE "SIGNAL_TICTACTOE"
#define SIGNAL_TTT_RESULT "SIGNAL_TTT_RESULT"
#define SIGNAL_TICTACTOE_AI "SIGNAL_TICTACTOE_AI"

// tictactoe ranking
#define SIGNAL_TTT_RANKING "SIGNAL_TTT_RANKING"

// server connect to client
int PORT;
struct sockaddr_in server_addr, client_addr;
fd_set master;
char send_msg[BUFF_SIZE], recv_msg[BUFF_SIZE];

// server variable
char token[] = "#";
char *str;
int tttResult;

void updateCaroRanking(char *user, int winOrLost)
{
  readFileCaroRanking();
  caronode *tmp = checkUserCaro(user);
  if (tmp == NULL)
  { // user ko co trong danh sach
    userInforCaro caroUser;
    strcpy(caroUser.username, user);
    caroUser.numberOfWin = 0;
    caroUser.numberOfLose = 0;
    caroUser.numberOfDraws = 0;
    caroUser.point = 0;
    insertCaro(caroUser);
    updateFileCaroRanking();
    tmp = checkUserCaro(user);
  }

  if (winOrLost == 1)
  { // 0 hòa, -1 thua, 1 thắng
    tmp->user.numberOfWin++;
    tmp->user.point++;
    updateFileCaroRanking();
  }
  else if (winOrLost == -1)
  {
    tmp->user.numberOfLose++;
    tmp->user.point--;
    updateFileCaroRanking();
  }
  else if (winOrLost == 0)
  {
    tmp->user.numberOfDraws++;
    tmp->user.point = tmp->user.point + 0.5;
    updateFileCaroRanking();
  }

  // traversingListCaro();
  caroroot = NULL;
  carocur = NULL;
  caronew = NULL;
  tmp = NULL;
}

int handleDataFromClient(int fd)
{
  char *user, *pass, *id;
  int recieved, col, row;
  ClientInfo *info;

  recieved = recv(fd, recv_msg, BUFF_SIZE, 0);
  recv_msg[recieved] = '\0';
  // printf("%s\n", recv_msg);
  str = strtok(recv_msg, token);
  if (strcmp(str, SIGNAL_CLOSE) == 0)
  {
    FD_CLR(fd, &master); // Clears the bit for the file descriptor fd in the file descriptor set fdset.
    printf("Close connection from fd = %d\n", fd);
  }
  else if (strcmp(str, SIGNAL_CREATEUSER) == 0)
  {
    // Create new user
    user = strtok(NULL, token);
    pass = strtok(NULL, token);
    if (isValid(user, NULL))
    {
      sprintf(send_msg, "%s#%s", SIGNAL_ERROR, "Account existed");
    }
    else
    {
      registerUser(user, pass);
      sprintf(send_msg, SIGNAL_OK);
    }

    send(fd, send_msg, strlen(send_msg), 0);
  }
  else if (strcmp(str, SIGNAL_CHECKLOGIN) == 0)
  {
    // Login
    user = strtok(NULL, token);
    pass = strtok(NULL, token);
    if (isValid(user, pass))
      strcpy(send_msg, SIGNAL_OK);
    else
      sprintf(send_msg, "%s#%s", SIGNAL_ERROR, "Username or Password is incorrect");
    // while(1); // test timeout
    send(fd, send_msg, strlen(send_msg), 0);
  }
  else if (strcmp(str, SIGNAL_CARO_NEWGAME) == 0)
  {
    // Play new game
    str = strtok(NULL, token); //get size
    recieved = atoi(str);
    str = strtok(NULL, token);                                    //get user
    id = addInfo(inet_ntoa(client_addr.sin_addr), recieved, str); //get id game
    printf("Caro game with id = %s\n", id);
    printf("Caro Game Info: ");
    printInfo(getInfo(id));
    sprintf(send_msg, "%s#%s", SIGNAL_OK, id);
    send(fd, send_msg, strlen(send_msg), 0);
  }
  else if (strcmp(str, SIGNAL_CARO_RANKING) == 0)
  {
    // Handle caro ranking
    str = strtok(NULL, token);
    id = str;
    printf("Caro Ranking with id = %s\n", id);
    // xu li phan gui thong tin so tran thang, thua, diem
    readFileCaroRanking();
    caronode *tmp = checkUserCaro(id);
    char inforUser[100];
    if (tmp == NULL)
      strcpy(inforUser, "-1");
    else
    {
      printf("Username-ID: %s, Win: %d, Lose: %d, Draws: %d, Point: %.1f\n", tmp->user.username, tmp->user.numberOfWin, tmp->user.numberOfLose, tmp->user.numberOfDraws, tmp->user.point);
      sprintf(inforUser, "%d#%d#%d#%f", tmp->user.numberOfWin, tmp->user.numberOfLose, tmp->user.numberOfDraws, tmp->user.point);
    }
    // traversingListCaro(); // duyet danh sach caroRanking in ra phia server
    caroroot = NULL;
    carocur = NULL;
    caronew = NULL;
    tmp = NULL;
    sprintf(send_msg, "%s#%s#%s", SIGNAL_OK, id, inforUser);
    send(fd, send_msg, strlen(send_msg), 0);
  }
  else if (strcmp(str, SIGNAL_CARO_TURN) == 0)
  {
    // Quay về server để xử lí game caro
    id = strtok(NULL, token);   // get game id
    user = strtok(NULL, token); // get user name

    str = strtok(NULL, token); // //get column
    col = atoi(str);

    str = strtok(NULL, token); //get row
    row = atoi(str);
    printf("Received a turn of game id = %s, user = %s : column = %d, row = %d\n", id, user, col, row);
    // set table
    info = getInfo(id);
    if (info != NULL)
    {
      setTable(info->table, info->size, -99, -100);
      // write log
      writeLog(info->logfile, col, row, 1);
      // player win
      if (playerMove(col, row) == 1)
      {
        printf("Player win\n");
        updateCaroRanking(user, 1); // update caro Ranking, win = 1
        strcpy(send_msg, SIGNAL_CARO_WIN);
        send(fd, send_msg, strlen(send_msg), 0);
      }
      else if (cpuMove(&col, &row) == 0)
      {
        //write log
        writeLog(info->logfile, col, row, 0);
        printf("Send a turn : column = %d, row = %d\n", col, row);
        sprintf(send_msg, "%s#%d#%d", SIGNAL_CARO_TURN, col, row);
        send(fd, send_msg, strlen(send_msg), 0);
      }
      else
      {
        //write log
        writeLog(info->logfile, col, row, 0);
        printf("Send a turn : column = %d, row = %d\n", col, row);
        updateCaroRanking(user, -1); // update caro Ranking, lost = -1
        sprintf(send_msg, "%s#%d#%d", SIGNAL_CARO_LOST, col, row);
        send(fd, send_msg, strlen(send_msg), 0);
      }
    }
    else
    {
      printf("Request a turn of game with id = %s REQUEST FAILED!\n", id);
      sprintf(send_msg, "%s#%s%s%s", SIGNAL_ERROR, "Game with id=", id, "not existed");
      send(fd, send_msg, strlen(send_msg), 0);
    }
  }
  else if (strcmp(str, SIGNAL_CARO_VIEWLOG) == 0)
  {
    // View log
    // get id-username
    id = strtok(NULL, token);
    info = getInfo(id);
    if (info != NULL)
    {
      printf("Request view log of caro game with id = %s | OK!\n", id);
      sprintf(send_msg, "%s#%s", SIGNAL_LOGLINE, info->logfile);
      send(fd, send_msg, strlen(send_msg), 0);
    }
    else
    {
      printf("Request view log of caro game with id = %s | FAILED!!!\n", id);
      printf("Game with id = %s not existed", id);
      sprintf(send_msg, "%s#%s%s%s", SIGNAL_ERROR, "Game with id=", id, "not existed");
      send(fd, send_msg, strlen(send_msg), 0);
    }
  }
  else if (strcmp(str, SIGNAL_CARO_ABORTGAME) == 0)
  {
    // hủy bỏ trò chơi
    id = strtok(NULL, token);
    str = strtok(NULL, token);
    if (removeInfo(id) == 0)
    {
      printf("Remove user %s's caro game with id = %s\n", str, id);
      strcpy(send_msg, SIGNAL_OK);
    }
    else
    {
      printf("Remove user %s's caro game with id = %s REQUEST FAILED!!!\n", str, id);
      printf("Caro game with id = %s not existed\n", id);
      sprintf(send_msg, "%s#%s%s%s", SIGNAL_ERROR, "Caro game with id=", id, "not existed");
    }
    send(fd, send_msg, strlen(send_msg), 0);
  }
}
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Syntax Error.\n");
    printf("Syntax: ./server PortNumber\n");
    return 0;
  }
  if (check_port(argv[1]) == 0)
  {
    printf("Port invalid\n");
    return 0;
  }
  PORT = atoi(argv[1]);

  int sock, connected, sin_size, true = 1;
  int fdmax, i, rc;
  fd_set read_fds;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  initList();

  // Step 1: Construct a TCP socket to listen connection request
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Socket Error!!!\n");
    exit(-1);
  }
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1)
  {
    perror("Setsockopt error!!!\n");
    exit(-2);
  }

  //Step 2: Bind address to socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(server_addr.sin_zero), 8);

  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
  {
    perror("Unable to bind\n");
    exit(-3);
  }

  //Step 3: Listen request from client
  if (listen(sock, 5) == -1)
  {
    perror("Listen error\n");
    exit(-4);
  }
  printf("ANHTNGAME waiting for client on port %d\n", PORT);
  fflush(stdout);

  FD_SET(sock, &master);
  fdmax = sock;

  // Set timeout
  struct timeval timeout;
  timeout.tv_sec = 1000; // after 1000 seconds will timeout
  timeout.tv_usec = 0;
  //Step 4: Communicate with clients
  while (1)
  {
    read_fds = master;
    rc = select(fdmax + 1, &read_fds, NULL, NULL, &timeout);
    if (rc == -1)
    {
      perror("select() error!\n");
      exit(-6);
    }
    else if (rc == 0)
    {
      printf("  select() timed out. End program.\n");
      exit(-5);
    }
    for (i = 0; i <= fdmax; i++)
    {
      if (FD_ISSET(i, &read_fds))
      {
        if (i == sock)
        {
          sin_size = sizeof(struct sockaddr_in);
          connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
          if (connected == -1)
          {
            perror("accept error!\n");
            exit(-7);
          }
          else
          {
            FD_SET(connected, &master);
            if (connected > fdmax)
              fdmax = connected;
            printf("Got a connection from (%s , %d) with fd = %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), connected);
            handleDataFromClient(connected);
          }
        }
        else
        {
          handleDataFromClient(i);
        }
      }
    }
  }
  close(sock);
  return 0;
}