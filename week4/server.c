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

NODE *userLoged = NULL;
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
		printf("Hello %s\n\n\n", user->x.userName);
		userLoged = user;
	}
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
	printf("\n\nYour file valid! Below is you data after insert:\n\n");
	DLIST(l);
}

int main()
{
	int sockfd;
	char username[MAXLINE];
	char password[MAXLINE];

	char *helloFromserver = "Connected ...Insert username and password..\n";
	char *endConnect = "end";
	char *next = "next";
	char *insertPassword = "Insert Password";

	struct sockaddr_in servaddr, cliaddr;

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

	socklen_t len;
	len = sizeof(cliaddr); //len is value/resuslt

	recvfrom(sockfd, (char *)username, MAXLINE,
			 MSG_WAITALL, (struct sockaddr *)&cliaddr,
			 &len);
	sendto(sockfd, (const char *)helloFromserver, strlen(helloFromserver),
		   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
		   len);

	while (1)
	{
		// Reset string receive
		memset(username, 0, sizeof(username));
		recvfrom(sockfd, (char *)username, MAXLINE,
				 MSG_WAITALL, (struct sockaddr *)&cliaddr,
				 &len);

		printf("Username : %s\n", username);

		if (strcmp(username, "bye") == 0)
		{
			sendto(sockfd, (const char *)endConnect, strlen(endConnect),
				   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
				   len);
			break;
		}
		sendto(sockfd, (const char *)insertPassword, strlen(insertPassword),
			   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
			   len);

		memset(password, 0, sizeof(password));

		recvfrom(sockfd, (char *)password, MAXLINE,
				 MSG_WAITALL, (struct sockaddr *)&cliaddr,
				 &len);

		printf("Password : %s\n", password);

		printf("Status after check: %d\n", loginAccount(l, username, password));
	}

	return 0;
}