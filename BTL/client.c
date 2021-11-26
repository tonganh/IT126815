/*
Members:
-----Tong Ngoc Anh-----(L)
-----Nguyen Thi Ngoc Diem-----
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
/*
Library of socket
*/
#include <unistd.h>
#include <sys/types.h>
//#include <winsock2.h>  //dung cho window
#include <sys/socket.h>
#include <arpa/inet.h>
/*
https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/rttcga.htm
https://www.daemon-systems.org/man/tcgetattr.3.html
*/
#include <termios.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>

#include "linklist.h"
#include "checkinput.h"

#define BUFF_SIZE 1024
int VIEWSIZE = 15;

#include "clientHelper.h"

// fungame
#define STATUS_START_MENU 0
#define STATUS_FUNGAME_MENU 1
#define STATUS_HANDLE_GAME 2
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
// show info
#define CARO_INFO_QUIT "Quit caro game (y or n)?"
#define CARO_INFO_WIN "You Win. Do you want to view log (y or n)?"
#define CARO_INFO_LOST "You Lose. Do you want to view log (y or n)?"

// caro ranking
#define SIGNAL_CARO_RANKING "SIGNAL_CARO_RANKING"

// tictactoe game
#define SIGNAL_TICTACTOE "SIGNAL_TICTACTOE"
#define SIGNAL_TTT_RESULT "SIGNAL_TTT_RESULT"
#define SIGNAL_TICTACTOE_AI "SIGNAL_TICTACTOE_AI"

// tictactoe ranking
#define SIGNAL_TTT_RANKING "SIGNAL_TTT_RANKING"

// client connect to server
struct sockaddr_in server_addr;
int PORT, sock, recieved, isCommunicating;
char *serverAddress;
char send_msg[BUFF_SIZE], recv_msg[BUFF_SIZE];

// client variable
int status; // status of fungame
char choice, token[] = "#";
char error[100], user[100], id[30];

// draw table
char *table;
int size, playerTurn, col, row;
int viewC = 0, viewR = 0;

// trang thai choi caro hay tic-tac-toe, 1 la caro, 2 la tic-tac-toe
int checkCaroOrTicTacToe;

// result of TicTacToe
int resultTTT;

/*
Kết nối với server
*/
int connectToServer()
{
    isCommunicating = 1; // tao ket noi
    int errorConnect;
    recieved = -1;
    //Step 1: Construct socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        strcpy(error, "Error Socket!!!");
        return -1;
    }

    //Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(serverAddress);

    // set timeout
    struct timeval timeout;
    timeout.tv_sec = 20; // after 20 seconds connect will timeout
    timeout.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        return -1;
    }
    else if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        return -1;
    }

    //Step 3: Request to connect server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        errorConnect = errno;
        sprintf(error, "Error! Can not connect to server! %s", strerror(errorConnect));
        return -1;
    }
    //Step 4: Communicate with server
    send(sock, send_msg, strlen(send_msg), 0);
    recieved = recv(sock, recv_msg, BUFF_SIZE, 0);
    recv_msg[recieved] = '\0';
    strcpy(send_msg, SIGNAL_CLOSE); // gui tin hieu ngat ket noi
    send(sock, send_msg, strlen(send_msg), 0);
    close(sock);
    if (recieved == -1)
    {
        printf("\nError: Timeout!!!\n");
        return -1;
    }
    isCommunicating = 0; // ngat ket noi
    return 0;
}

/*
Hiển thị phần login
*/
int menuSignin()
{
    error[0] = '\0';
    char pass[100], *str;
    while (1)
    {
        clearScreen();
        printf("----------FUNGAME----------\n");
        printf("\tSign in\n");
        printf("---------------------------\n");
        if (error[0] != '\0')
        {
            printf("Error: %s!\n", error);
            printf("Do you want to try again?(y or n): ");
            choice = getchar();
            while (getchar() != '\n')
                ;
            if (choice == 'n' || choice == 'N')
            {
                error[0] = '\0';
                return -1;
            }
            else if (choice != 'y' && choice != 'Y')
            {
                continue;
            }
            else
            {
                error[0] = '\0';
                continue;
            }
        }
        printf("Username: ");
        fgets(user, BUFF_SIZE, stdin);
        user[strlen(user) - 1] = '\0';

        printf("Password: ");
        getPassword(pass);

        //check username and password
        sprintf(send_msg, "%s#%s#%s", SIGNAL_CHECKLOGIN, user, pass);
        if (connectToServer() == 0)
        {
            str = strtok(recv_msg, token);
            if (strcmp(str, SIGNAL_OK) == 0)
            {
                // break;
                return 0;
            }
            else if (strcmp(str, SIGNAL_ERROR) == 0)
            {
                str = strtok(NULL, token);
                strcpy(error, str);
                return -2;
            }
        }
        else
        {
            printf("Error! Cant connect to server!\n");
            return -1;
        }
    }
    // return 0;
}

/*
Hiển thị phần đăng kí
*/
int menuRegister()
{
    error[0] = '\0';
    char pass[100], comfirmPass[100], *str;
    while (1)
    {
        clearScreen();
        printf("----------FUNGAME----------\n");
        printf("\tRegister\n");
        printf("---------------------------\n");
        if (error[0] != '\0')
        {
            printf("Error: %s!\n", error);
            printf("Do you want to try again?(y or n): ");
            choice = getchar();
            while (getchar() != '\n')
                ;
            if (choice == 'n' || choice == 'N')
            {
                error[0] = '\0';
                return -2;
            }
            else if (choice != 'y' && choice != 'Y')
                continue;
            else
            {
                error[0] = '\0';
                continue;
            }
        }
        printf("Username: ");
        fgets(user, BUFF_SIZE, stdin);
        user[strlen(user) - 1] = '\0';

        printf("Password: ");
        getPassword(pass);
        printf("\nConfirm password: ");
        getPassword(comfirmPass);
        if (strcmp(pass, comfirmPass) == 0)
        {
            // register new account
            sprintf(send_msg, "%s#%s#%s", SIGNAL_CREATEUSER, user, pass);
            if (connectToServer() == 0)
            {
                str = strtok(recv_msg, token);
                if (strcmp(str, SIGNAL_OK) == 0)
                    break;
                else if (strcmp(str, SIGNAL_ERROR) == 0)
                {
                    str = strtok(NULL, token);
                    strcpy(error, str);
                }
            }
            else
            {
                printf("Error! Cant connect to server!\n");
                return -1;
            }
        }
        else
        {
            strcpy(error, "Password does not match");
        }
    }
    return 0;
}

/*
Hiển thị chọn login, create
*/
int menuStart()
{
    int checkSignin = 0, checkRegister = 0;
    error[0] = '\0';
    while (1)
    {
        clearScreen();
        if (error[0] != '\0')
        {
            printf("Error: %s!\n", error);
            error[0] = '\0';
        }
        printf("----------FUNGAME----------\n");
        printf("\t1.Sign in\n");
        printf("\t2.Register\n");
        printf("\t3.Exit\n");
        printf("---------------------------\n");
        printf("Your choice: ");
        scanf("%c", &choice);
        while (getchar() != '\n')
            ;
        if (choice == '1')
        {
            checkSignin = menuSignin();
            if (checkSignin == 0)
            {
                break;
            }
            else if (checkSignin == -1)
                return -1;
        }
        else if (choice == '2')
        {
            checkRegister = menuRegister();
            if (checkRegister == 0)
                break;
            else if (checkRegister == -1)
                return -1;
        }
        else if (choice == '3')
            return -1;
        else
            sprintf(error, "No option %c", choice);
    }

    return 0;
}

int menuFunGame()
{
    error[0] = '\0';
    while (1)
    {
        clearScreen();
        if (error[0] != '\0')
        {
            printf("Error: %s!\n", error);
            error[0] = '\0';
        }
        printf("----------FUNGAME----------\n");
        printf("\033[0;33m\tWelcome %s \033[0;37m\n", user);
        printf("\t1.Caro game\n");
        printf("\t2.TicTacToe game\n");
        printf("\t3.Caro ranking\n");
        printf("\t4.TicTacToe ranking\n");
        printf("\t5.Sign out\n");
        printf("---------------------------\n");
        printf("Your choice: ");
        scanf("%c", &choice);
        while (getchar() != '\n')
            ;
        if (choice == '1')
        {
            checkCaroOrTicTacToe = 1;
            break;
        }
        else if (choice == '2')
        {
            printf("hello!\n");
            break;
        }
        else if (choice == '3')
        {
            checkCaroOrTicTacToe = 3;
            break;
        }
        else if (choice == '4')
        {
            checkCaroOrTicTacToe = 4;
            break;
        }
        else if (choice == '5')
        {
            return -1;
        }
        else
        {
            sprintf(error, "%c Not an optional", choice);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int err;
    if (argc != 3)
    {
        printf("Syntax Error.\n");
        printf("Syntax: ./client IPAddress PortNumber\n");
        return 0;
    }
    if (check_IP(argv[1]) == 0)
    {
        printf("IP address invalid\n");
        return 0;
    }
    if (check_port(argv[2]) == 0)
    {
        printf("Port invalid\n");
        return 0;
    }
    serverAddress = argv[1];
    PORT = atoi(argv[2]);

    status = STATUS_START_MENU;
    while (1)
    {
        if (status == STATUS_START_MENU)
        {
            if (menuStart() == -1)
                break;
            else
                status = STATUS_FUNGAME_MENU;
        }
        else if (status == STATUS_FUNGAME_MENU)
        {
            if (menuFunGame() == -1)
                status = STATUS_START_MENU;
            else
                status = STATUS_HANDLE_GAME;
        }
        else if (status == STATUS_HANDLE_GAME)
        {
            if (checkCaroOrTicTacToe == 1)
            {
                free(table);
                table = NULL;
                status = STATUS_FUNGAME_MENU;
            }
            else if (checkCaroOrTicTacToe == 2)
            {
                printf("hello!\n");
            }
            else if (checkCaroOrTicTacToe == 3)
            {
                status = STATUS_FUNGAME_MENU;
            }
            else if (checkCaroOrTicTacToe == 4)
            {
                status = STATUS_FUNGAME_MENU;
            }
        }
    }
    return 0;
}