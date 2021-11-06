#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 819600
#define WINDOW_SIZE 102400

char *recv_msg(int conn_sock)
{
	int ret, nLeft, msg_len, index = 0;
	char recv_data[WINDOW_SIZE], *data;
	// receive the length of message
	int bytes_received = recv(conn_sock, &msg_len, sizeof(int), MSG_WAITALL);
	if (bytes_received <= 0)
	{
		return NULL;
	}
	nLeft = msg_len;
	data = (char *)malloc(msg_len);
	memset(data, 0, msg_len);
	index = 0;

	//receives message from server
	while (nLeft > 0)
	{
		ret = recv(conn_sock, recv_data, WINDOW_SIZE, 0);
		if (ret == -1)
		{
			break;
		}
		memcpy(data + index, recv_data, ret);
		index += ret;
		nLeft -= ret;
	}
	data[msg_len] = '\0';
	return data;
}

int send_msg(int conn_sock, char *message, int msg_len)
{
	int bytes_sent;
	//send the length of the message to server
	bytes_sent = send(conn_sock, &msg_len, sizeof(int), 0);
	if (bytes_sent <= 0)
	{
		return -1;
	}

	// send the message to server
	bytes_sent = send(conn_sock, message, msg_len, 0);
	if (bytes_sent <= 0)
	{
		return -1;
	}
	return 0;
}

int send_eof_msg(int conn_sock)
{
	int msg_len = 0;
	int bytes_sent = send(conn_sock, &msg_len, sizeof(int), 0);
	if (bytes_sent <= 0)
	{
		return -1;
	}
	return 0;
}

void welcome()
{
	printf("TCP File Transfer Application\n");
	printf("Nguyen Duc Hieu - 20184099\n");
	printf("\nFile Location: ");
}

int main(int argc, char *argv[])
{
	int client_sock;
	char buff[BUFF_SIZE], filelink[BUFF_SIZE], filename[BUFF_SIZE], *data;
	struct sockaddr_in server_addr;
	int serv_port = 0;
	char serv_ip[16];
	char *endptr;
	FILE *fp = NULL;
	int errnum;
	double bytes_transfered = 0;
	if (argc != 3)
	{
		printf("Invalid arguments!\n");
		exit(-1);
	}

	strcpy(serv_ip, argv[1]);
	serv_port = (in_port_t)strtol(argv[2], &endptr, 10);
	if (strlen(endptr) != 0)
	{
		printf("Invalid port!\n");
		exit(-1);
	}

	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serv_port);
	server_addr.sin_addr.s_addr = inet_addr(serv_ip);

	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

	while (1)
	{
		bytes_transfered = 0;
		welcome();
		memset(buff, '\0', BUFF_SIZE);
		fgets(buff, BUFF_SIZE, stdin);
		buff[strlen(buff) - 1] = '\0';
		if (strlen(buff) == 0)
			break;
		strcpy(filelink, buff);

		if ((fp = fopen(filelink, "rb")) == NULL)
		{
			printf("Error: File not found\n");
			continue;
		}
		else
		{
			strcpy(filename, rindex(filelink, '/') + 1);
			if (send_msg(client_sock, filename, strlen(filename)) == -1)
			{
				continue;
			}
			data = recv_msg(client_sock);
			errnum = atoi(data);
			if (errnum == 1)
			{
				printf("Error: File is existent on server\n");
			}
			else if (errnum == 0)
			{
				while (errnum == 0)
				{
					memset(buff, '\0', BUFF_SIZE);
					if (fread(buff, BUFF_SIZE, 1, fp) == 1)
					{

						if (send_msg(client_sock, buff, sizeof(buff)) == -1)
						{
							break;
						}
						printf("Transfered: %.2lf MB\n", (bytes_transfered += sizeof(buff)) / (1024 * 1024));
					}
					else
					{
						if (send_msg(client_sock, buff, sizeof(buff)) == -1)
						{
							break;
						}
						printf("Transfered: %.2lf MB\n", (bytes_transfered += sizeof(buff)) / (1024 * 1024));
						data = recv_msg(client_sock);
						errnum = atoi(data);
						if (send_eof_msg(client_sock) == -1)
						{
							break;
						}
					}
					data = recv_msg(client_sock);
					errnum = atoi(data);
				}
				if (errnum == -1)
				{
					printf("\nSuccessful transfering\n");
				}
				else
				{
					printf("\nError: File transfering is interupted\n");
				}
			}
		}
		fclose(fp);
	}
	close(client_sock);
	return 0;
}
