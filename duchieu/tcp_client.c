#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5500
#define BUFF_SIZE 1024

char buffer[BUFF_SIZE + 1];
char mes[BUFF_SIZE + 1];

// int xuli()
// {
// 	if (strstr(buffer, ".txt") == NULL)
// 	{

// 		printf("Error: Wrong file format\n");

// 		return 1;
// 	}
// 	FILE *fp = fopen(buffer, "r");
// 	if (fp == NULL)
// 	{
// 		printf("khong co file\n");
// 		return 3;
// 	}
// 	else
// 	{
// 		fclose(fp);
// 	}

// 	int k = 0;
// 	strcpy(mes, "file/");
// 	for (int i = 0; i < strlen(buffer); i++)
// 	{
// 		if (buffer[i] == '/')
// 		{
// 			for (int j = i; j < strlen(buffer) - 1; j++)
// 			{
// 				buffer[j] = buffer[j + 1];
// 			}
// 			buffer[strlen(buffer) - 1] = '\0';
// 		}
// 	}
// 	strcat(mes, buffer);
// 	fp = fopen(mes, "r");
// 	if (fp != NULL)
// 	{
// 		printf("file da ton tai\n");
// 		fclose(fp);
// 		return 2;
// 	}

// 	return 0;
// }

int main()
{
	int client_sock;

	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
	printf("\nInsert path of file to send:");

	fgets(buffer, BUFF_SIZE, stdin);
	msg_len = strlen(buffer);
	bytes_sent = send(client_sock, buffer, msg_len, 0);
	if (bytes_sent < 0)
		perror("\nError: ");

	memset(buffer, '\0', (strlen(buffer) + 1));
	bytes_received = recv(client_sock, buffer, BUFF_SIZE, 0);
	printf("%s\n", buffer);

	// if (bytes_received < 0)
	// 	perror("\nError: ");
	// else if (bytes_received == 0)
	// 	printf("Connection closed.\n");

	// buffer[bytes_received] = '\0';
	// printf("Reply from server: %s", buffer);

	//Step 4: Close socket
	close(client_sock);
	return 0;

	//receive echo reply
}