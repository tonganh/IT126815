// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "stack.h"

#define PORT 8080
#define MAXLINE 1024
#define SUCCESSFULL 1
#define ERROR 0

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

void actionAfterChangePassword(char *newPassword, LIST *l)
{
	int changePasswordStatus = changePassword(newPassword, l);

	// In case have special character => out => send error to client
	if (changePasswordStatus == ERROR)
	{
		sendto(sockfd, (const char *)"Fail", strlen("Fail"),
			   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
			   len);
	}
	else
	{

		// Send message
		sendto(sockfd, (const char *)"SuccessFull", strlen("SuccessFull"),
			   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
			   len);

		sendto(sockfd, (const char *)passwordAlpha, strlen(passwordAlpha),
			   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
			   len);

		sendto(sockfd, (const char *)passwordDigit, strlen(passwordDigit),
			   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
			   len);
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

int main(int argc, char *argv[])
{

	if (argc < 2)
	{
		printf("Invalid argument. The model using is: ./server {{PORT}}\n");
		exit(0);
	}
	int port = atoi(argv[1]);

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
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *)&servaddr,
			 sizeof(servaddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	char firstHandle[MAXLINE];
	recvfrom(sockfd, (char *)firstHandle, MAXLINE,
			 MSG_WAITALL, (struct sockaddr *)&cliaddr,
			 &len);

	sendto(sockfd, (const char *)helloFromserver, strlen(helloFromserver),
		   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
		   len);

	while (1)
	{
		// Reset string receive
		memset(username, 0, sizeof(username));
		// Recv username
		recvfrom(sockfd, (char *)username, MAXLINE,
				 MSG_WAITALL, (struct sockaddr *)&cliaddr,
				 &len);

		if (strcmp(username, "") == 0)
		{
			printf("Turn off server...\n");
			exit(0);
		}

		sendto(sockfd, (const char *)insertPassword, strlen(insertPassword),
			   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
			   len);

		memset(password, 0, sizeof(password));

		recvfrom(sockfd, (char *)password, MAXLINE,
				 MSG_WAITALL, (struct sockaddr *)&cliaddr,
				 &len);

		int i_valueAfterLogin = loginAccount(l, username, password);
		// Check account after submit username and password
		char *test2 = convertValue(i_valueAfterLogin);
		sendto(sockfd, (const char *)test2, strlen(test2),
			   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
			   len);

		char *activeStatus = "1";
		if (strcmp(test2, activeStatus) == 0)
		{
			char passwordWantChange[MAXLINE];
			memset(passwordWantChange, 0, sizeof(passwordWantChange));
			// 1st time
			recvfrom(sockfd, (char *)passwordWantChange, MAXLINE,
					 MSG_WAITALL, (struct sockaddr *)&cliaddr,
					 &len);

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
				actionAfterChangePassword(passwordWantChange, l);
				// Reset string and listen for next action
				memset(passwordWantChange, 0, sizeof(passwordWantChange));
				recvfrom(sockfd, (char *)passwordWantChange, MAXLINE,
						 MSG_WAITALL, (struct sockaddr *)&cliaddr,
						 &len);
			}

			if (strcmp(passwordWantChange, "bye") == 0)
			{
				userLoged = NULL;
			}
		}
	}

	return 0;
}