#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 5500
#define BACKLOG 20
#define BUFF_SIZE 1024

/* Handler process signal*/
void sig_chld(int signo);

/*
* Receive and echo message to client
* [IN] sockfd: socket descriptor that connects to client 	
*/
void echo(int sockfd);
int xuli();
int docfile();
char mes[BUFF_SIZE], buffer[BUFF_SIZE];

int main()
{

	int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server;	/* server's address information */
	struct sockaddr_in client;	/* client's address information */
	pid_t pid;
	int sin_size;
	int opt = 1;

	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ /* calls socket() */
		printf("socket() error\n");
		return 0;
	}
	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

	if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	if (listen(listen_sock, BACKLOG) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);
	while (1)
	{
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
		{
			if (errno == EINTR)
				continue;
			else
			{
				perror("\nError: ");
				return 0;
			}
		}
		pid = fork();
		if (pid == 0)
		{
			close(listen_sock);
			printf("You got a connection from %s\n", inet_ntoa(client.sin_addr));
			echo(conn_sock);
			exit(0);
		}
		close(conn_sock);
	}
	close(listen_sock);
	return 0;
}

void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("\nChild %d terminated\n", pid);
}

char mes1[BUFF_SIZE];
int xuli()
{
	if (strstr(buffer, ".txt") == NULL)
	{

		strcpy(mes1, "Error: Wrong file format");

		return 1;
	}
	FILE *fp = fopen(buffer, "r");
	if (fp == NULL)
	{
		printf("%s\n", buffer);
		strcpy(mes1, "khong co file");
		return 3;
	}
	else
	{
		fclose(fp);
	}

	int k = 0;
	strcpy(mes, "file/");
	for (int i = 0; i < strlen(buffer); i++)
	{
		if (buffer[i] == '/')
		{
			for (int j = i; j < strlen(buffer) - 1; j++)
			{
				buffer[j] = buffer[j + 1];
			}
			buffer[strlen(buffer) - 1] = '\0';
		}
	}
	strcat(mes, buffer);

	fp = fopen(mes, "rb");
	if (fp != NULL)
	{
		strcpy(mes1, "file da ton tai");
		fclose(fp);
		return 2;
	}
	strcpy(mes1, "ok");
	return 0;
}
void echo(int sockfd)
{

	int bytes_sent, bytes_received;
	memset(buffer, '\0', (strlen(buffer) + 1));
	bytes_received = recv(sockfd, buffer, BUFF_SIZE, 0); //
	if (bytes_received < 0)
		perror("\nError: ");
	else if (bytes_received == 0)
		printf("Connection closed.");
	buffer[bytes_received - 1] = '\0';
	printf("%sDD\n", buffer);

	// memset(buffer, '\0', (strlen(buffer) + 1));
	// strcpy(buffer, "/home/hieudz/Desktop/test/1_1.txt");

	xuli();
	printf("%s\n", mes1);
	bytes_sent = send(sockfd, mes1, strlen(mes1), 0);
	if (bytes_sent < 0)
		perror("\nError: ");
	close(sockfd);
}