#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "stack.h"

#define MAXLINE 1024
#define SUCCESSFULL 1
#define ERROR 0
#define SA struct sockaddr

NODE *userLoged = NULL;
int checkingStatus = 0;
char passwordDigit[MAXLINE];
char passwordAlpha[MAXLINE];

struct sockaddr_in servaddr, cliaddr;
int sockfd;
socklen_t len = sizeof(cliaddr);
int firstCheckInput = 0;
void DNODE(DT x)
{
	printf("%-30s%-20s%-15d%-10d\n", x.userName, x.password, x.status, x.totalTimeWrongCode);
}

void DLIST(LIST *l)
{
	NODE *p = l->Head;
	printf("%-35s%-20s%-15s%-10s\n", "Tên đăng nhập", "Mật khẩu", "Trạng thái", "Số lần đã nhập sai");
	while (p != NULL)
	{
		DNODE(p->x);
		p = p->next;
	}
}
void Empty(LIST *l)
{
	l->Head = NULL;
}

void writeFileAfterUpdate(LIST *l)
{
	FILE *fptr = fopen("nguoidung.txt", "w");
	NODE *p = l->Head;
	while (p != NULL)
	{

		fwrite(p->x.userName, 1, strlen(p->x.userName), fptr);
		fwrite("\t", 1, 1, fptr);

		fwrite(p->x.password, 1, strlen(p->x.password), fptr);
		fwrite("\t", 1, 1, fptr);

		char convertTest[5];
		sprintf(convertTest, "%d", p->x.status);
		fwrite(convertTest, 1, strlen(convertTest), fptr);
		if (p->next != NULL)
		{
			fwrite("\n", 1, 1, fptr);
		}
		p = p->next;
	}
	fclose(fptr);
}

NODE *findAnAccount(LIST *l, char userName[100])
{
	NODE *p = l->Head;
	while (p != NULL)
	{
		if (strcmp(userName, p->x.userName) == 0)
		{
			return p;
			break;
		}
		p = p->next;
	}
	return NULL;
}

char *convertValue(long status)
{
	switch (status)
	{
	case ACTIVE:
		return "1";
		break;

	case NOT_EXIST:
		return "3";
		break;

	case BLOCKED:
		return "0";
		break;

	case WRONG_PASSWORD:
		return "5";
		break;

	default:
		return "0";
		break;
	}
}

int encodePassword(char *password)
{

	int index1 = 0, index2 = 0;
	memset(passwordAlpha, 0, sizeof(passwordAlpha));
	memset(passwordDigit, 0, sizeof(passwordDigit));

	for (int i = 0; i < strlen(password); i++)
	{
		// Check digit case
		if (isdigit(password[i]))
			passwordDigit[index1++] = password[i];
		else if ((password[i] >= 'a' && password[i] <= 'z') || (password[i] >= 'A' && password[i] <= 'Z'))
			passwordAlpha[index2++] = password[i];
		else
		{
			// Wrong case
			return 0;
		}
	}
	passwordDigit[index1] = '\0';
	passwordAlpha[index2] = '\0';

	return 1;
}

int changePassword(char *newPassword, LIST *l)
{

	if (!encodePassword(newPassword))
	{
		return ERROR;
	}
	strcpy(userLoged->x.password, newPassword);
	writeFileAfterUpdate(l);
	return SUCCESSFULL;
}

void actionAfterChangePassword(int connect_status, char *newPassword, LIST *l)
{
	int changePasswordStatus = changePassword(newPassword, l);

	printf("newPassword: %s\n", newPassword);
	// In case have special character => out => send error to client
	if (changePasswordStatus == ERROR)
	{
		printf("ERROR\n");
		write(connect_status, (const char *)"Fail", strlen("Fail"));
	}
	else
	{
		write(connect_status, (const char *)"successful", strlen("successful"));

		printf("changePasswordStatus:%d\n", changePasswordStatus);
		char buff[MAXLINE];
		bzero(buff, MAXLINE);
		// Send message

		if (strlen(passwordAlpha) == 0)
		{
			strcpy(passwordAlpha, " ");
		}
		write(connect_status, (const char *)passwordAlpha, strlen(passwordAlpha));

		read(connect_status, buff, MAXLINE);
		printf("buff:%s\n", buff);
		if (strlen(buff) != 0)
		{

			if (strlen(passwordDigit) == 0)
			{
				strcpy(passwordDigit, " ");
			}
			write(connect_status, (const char *)passwordDigit, strlen(passwordDigit));
		}
	}
}
int loginAccount(LIST *l, char *userName, char *password)
{

	NODE *user = findAnAccount(l, userName);
	if (user == NULL)
	{
		return NOT_EXIST;
	}

	if (user->x.status == BLOCKED)
	{
		return BLOCKED;
	}

	if (user->x.totalTimeWrongPassword == 3)
	{
		user->x.status = BLOCKED;
		writeFileAfterUpdate(l);
		return BLOCKED;
	}

	if (strcmp(user->x.password, password) == 0)
	{
		if (user->x.status != ACTIVE)
		{
			return BLOCKED;
		}
		user->x.totalTimeWrongPassword = 0;
		userLoged = user;
		return ACTIVE;
	}
	user->x.totalTimeWrongPassword++;
	return WRONG_PASSWORD;
}

// Driver code

void readFile(FILE *f, LIST *l)
{
	int lineCheck = 1;
	while (!feof(f))
	{
		DT x;
		x.totalTimeWrongCode = 0;
		x.totalTimeWrongPassword = 0;

		fscanf(f, "%s %s %d", x.userName, x.password, &x.status);
		NODE *checkAccount = findAnAccount(l, x.userName);
		if (checkAccount != NULL)
		{
			printf("\n======================================\n");
			printf("Username input in line %d have existed. Check you file!\n", lineCheck);
			printf("======================================\n");
			exit(0);
		}
		Push(l, x);
		lineCheck++;
	}
	// printf("\n\nYour file valid! Below is you data after insert:\n\n");
	// DLIST(l);
}

void func(int sockfd)
{
	char buff[MAXLINE];
	int n;
	// infinite loop for chat
	for (;;)
	{
		bzero(buff, MAXLINE);

		// read the message from client and copy it in buffer
		read(sockfd, buff, sizeof(buff));
		// print buffer which contains the client contents
		printf("From client: %s\t To client : ", buff);
		bzero(buff, MAXLINE);
		n = 0;
		// copy server message in the buffer
		while ((buff[n++] = getchar()) != '\n')
			;

		// and send that buffer to client
		write(sockfd, buff, sizeof(buff));

		// if msg contains "Exit" then server exit and chat ended.
		if (strncmp("exit", buff, 4) == 0)
		{
			printf("Server Exit...\n");
			break;
		}
	}
}

int main(int argc, char *argv[])
{

	if (argc < 2)
	{
		printf("Invalid argument. The model using is: ./server {{PORT}}\n");
		exit(0);
	}
	int port = atoi(argv[1]);

	int sockfd, connect_status;
	struct sockaddr_in servaddr, cli;

	char username[MAXLINE];
	char password[MAXLINE];

	char *helloFromserver = "Connected ...Insert username and password..\n";
	char *insertPassword = "Insert Password";

	FILE *fp;
	LIST *l = (LIST *)malloc(sizeof(LIST));
	Empty(l);
	fp = fopen("nguoidung.txt", "r+");
	if (fp == NULL)
	{
		perror("Can not open this file. Check again your file name.");
		exit(0);
	}
	readFile(fp, l);

	fclose(fp);

	// int n = 0;

	// Creating socket file descriptor
	printf("Server started and listen on PORT: %d\n", port);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *)&servaddr,
			 sizeof(servaddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connect_status = accept(sockfd, (SA *)&cli, &len);
	if (connect_status < 0)
	{
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");
	char firstHandle[MAXLINE];
	//
	// func(connect_status);
	//
	read(connect_status, firstHandle, MAXLINE);
	printf("Have any receiving: %s\n", firstHandle);
	write(connect_status, (const char *)helloFromserver, strlen(helloFromserver));

	while (1)
	{
		// Reset string receive
		memset(username, 0, sizeof(username));
		// Recv username
		read(connect_status, (char *)username, MAXLINE);

		if (strcmp(username, "") == 0)
		{
			printf("Turn off server...\n");
			exit(0);
		}

		write(connect_status, (const char *)insertPassword, strlen(insertPassword));

		memset(password, 0, sizeof(password));

		read(connect_status, (char *)password, MAXLINE);

		int i_valueAfterLogin = loginAccount(l, username, password);
		// Check account after submit username and password
		char *test2 = convertValue(i_valueAfterLogin);
		write(connect_status, (const char *)test2, strlen(test2));

		char *activeStatus = "1";
		if (strcmp(test2, activeStatus) == 0)
		{
			char passwordWantChange[MAXLINE];
			memset(passwordWantChange, 0, sizeof(passwordWantChange));
			// 1st time
			read(connect_status, (char *)passwordWantChange, MAXLINE);
			// char currentPos[100];
			// read(connect_status, (char *)currentPos, MAXLINE);
			// write(connect_status, (const char *)"kakaka", strlen("kakaka"));

			if (strcmp(passwordWantChange, "") == 0)
			{
				exit(0);
			}
			while (strcmp(passwordWantChange, "bye") != 0)
			{
				if (strcmp(passwordWantChange, "") == 0)
				{
					exit(0);
				}
				// char buff[MAXLINE];
				// read(connect_status, (char *)buff, MAXLINE);
				// bzero(buff, MAXLINE);

				actionAfterChangePassword(connect_status, passwordWantChange, l);
				// Reset string and listen for next action
				memset(passwordWantChange, 0, sizeof(passwordWantChange));
				read(connect_status, (char *)passwordWantChange, MAXLINE);
			}

			if (strcmp(passwordWantChange, "bye") == 0)
			{
				userLoged = NULL;
			}
		}
	}

	return 0;
}