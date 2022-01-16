#include "single.h"
#include "singleC.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_LEN 1024
#define CHECK 3
struct sockaddr_in server, client;
socklen_t slen = sizeof(client);
int max_sd, new_socket;

int checkString(char *str)
{
    if (strlen(str) == 0)
        return 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (!isalnum(str[i]))
            return 0;
    }
    return 1;
}
int convertStringToNumber(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
            return -1;
    }
    return atoi(str);
}
void ReadFile(char *filename, LIST *l)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Cannot find file %s\n", filename);
        exit(0);
    }
    int flag = 0;
    if (isEmpty(l))
        flag = 1;
    while (!feof(file))
    {
        DT a;
        fscanf(file, "%s", a.username);
        fscanf(file, "%s", a.password);
        fscanf(file, "%d", &a.status);
        fscanf(file, "%s", a.homepage);
        if (flag == 1)
        {
            AddTail(l, a);
        }
        else
        {
            Update(l, a);
        }
    }
    fclose(file);
}
void ReadFileConnect(char *filename, TREE *tree)
{
    FreeTREE(tree);
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Cannot find file %s\n", filename);
        exit(0);
    }
    while (!feof(file))
    {
        DATA a;
        fscanf(file, "%d", &a.socket_status);
        fscanf(file, "%s", a.username);
        if (!feof(file))
            AddTailTREE(tree, a);
    }
    fclose(file);
}
void WriteTREE(char *filename)
{
    FILE *file = fopen(filename, "w");
    fclose(file);
}
void WriteFile(LIST *l, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("Cannot find file %s\n", filename);
        exit(0);
    }
    NODE *p = l->Head;
    while (p != NULL)
    {
        fprintf(file, "%s %s %d %s\n", p->x.username, p->x.password, p->x.status, p->x.homepage);
        p = p->next;
    }
    fclose(file);
}
void WriteFileConnect(TREE *l, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("Cannot find file %s\n", filename);
        exit(0);
    }
    LEAF *p = l->Head;
    while (p != NULL)
    {
        fprintf(file, "%d %s\n", p->x.socket_status, p->x.username);
        p = p->next;
    }
    fclose(file);
}
void SendTo(int socket_status, char *messages)
{
    char data[MAX_LEN];
    strcpy(data, messages);
    if (write(socket_status, data, sizeof(data)) == -1)
    {
        printf("Error sendto.\n");
    }
}
void checkPassword(int socket_status, char *password, int *status, char *username, LIST *l, TREE *tree)
{
    NODE *login = FindByUsername(l, username);
    if (strcmp(login->x.password, password) == 0)
    {
        SendTo(socket_status, "OK");
        login->x.number_sign = 0;
        *status = 1;
        DATA a;
        strcpy(a.username, username);
        a.socket_status = socket_status;
        ReadFileConnect("connect.txt", tree);
        AddTailTREE(tree, a);
        WriteFileConnect(tree, "connect.txt");
    }
    else
    {
        if (login->x.number_sign == CHECK - 1)
        {
            login->x.status = 0;
            SendTo(socket_status, "Account is blocked");
            *status = 0;
        }
        else
        {
            login->x.number_sign++;
            SendTo(socket_status, "Not OK");
        }
    }
}
void splitString(char *str, char *result)
{
    char s1[MAX_LEN];
    char s2[MAX_LEN];
    int index1 = 0, index2 = 0;
    for (int i = 0; i < strlen(str); i++)
        if (isdigit(str[i]))
            s1[index1++] = str[i];
        else
            s2[index2++] = str[i];
    s1[index1] = '\0';
    s2[index2] = '\0';
    strcat(result, strcat(s1, s2));
}
void changePassword(int socket_status, char *password, int *status, char *username, LIST *l, TREE *tree)
{
    char data[MAX_LEN];
    NODE *login = FindByUsername(l, username);
    memset(data, '\0', MAX_LEN);
    if (strcmp(password, "bye") == 0)
    {
        *status = 0;
        ReadFileConnect("connect.txt", tree);
        Delete(tree, socket_status);
        WriteFileConnect(tree, "connect.txt");
        strcpy(data, "Goodbye ");
        strcat(data, login->x.username);
        SendTo(socket_status, data);
    }
    else if (!checkString(password))
        SendTo(socket_status, "Error");
    else
    {
        splitString(password, data);
        strcpy(login->x.password, password);
        SendTo(socket_status, data);
        ReadFileConnect("connect.txt", tree);
        LEAF *p = tree->Head;
        while (p != NULL)
        {
            if (strcmp(p->x.username, username) == 0 && p->x.socket_status != socket_status)
            {
                char mess[MAX_LEN];
                strcpy(mess, "Password changed. New Password: ");
                strcat(mess, password);
                SendTo(p->x.socket_status, mess);
            }
            p = p->next;
        }
    }
}
int send_(int *client_socket, int *client_status, char client_username[][MAX_LEN], int index, LIST *l, TREE *tree)
{
    int connect_status = client_socket[index];
    int status = client_status[index];
    char data[MAX_LEN];
    NODE *login;
    char username[MAX_LEN];
    strcpy(username, client_username[index]);
    bzero(data, MAX_LEN);
    if (read(connect_status, data, MAX_LEN) < 0)
    {
        printf("Error read.\n");
        exit(0);
    }
    if (strcmp(data, "") == 0)
    {
        client_socket[index] = 0;
        Delete(tree, connect_status);
        return 0;
    }
    printf("%d\n", status);
    printf("Client %d send: %s\n", connect_status, data);
    ReadFile("nguoidung.txt", l);
    switch (status)
    {
    case 0:
        login = FindByUsername(l, data);
        if (login)
        {
            if (login->x.status != 1)
            {
                SendTo(connect_status, "Account not ready");
                status = 0;
            }
            else
            {
                SendTo(connect_status, "Insert password");
                status = 2;
                strcpy(username, login->x.username);
            }
        }
        else
            SendTo(connect_status, "Wrong account");
        break;
    case 1:
        changePassword(connect_status, data, &status, username, l, tree);
        break;
    case 2:
        puts(username);
        checkPassword(connect_status, data, &status, username, l, tree);
        break;
    default:
        printf("Error status \n");
        exit(0);
    }
    client_status[index] = status;
    strcpy(client_username[index], username);
    WriteFile(l, "nguoidung.txt");
    WriteFileConnect(tree, "connect.txt");
    return 1;
}
int main(int argc, char *argv[])
{
    WriteTREE("connect.txt");
    if (argc != 2)
    {
        printf("Input error!\n./server PortNumber\n");
        return 0;
    }
    int portNumber = convertStringToNumber(argv[1]);
    if (portNumber < 0 || portNumber > 65535)
    {
        printf("PortNumber: integer from 0 to 65535\n");
        return 0;
    }
    char data[MAX_LEN];
    int socket_status;
    fd_set readfds;
    int client_socket[30], max_clients = 30;
    int client_status[30];
    char client_username[30][MAX_LEN];
    LIST *l = (LIST *)malloc(sizeof(LIST));
    TREE *tree = (TREE *)malloc(sizeof(TREE));

    for (int i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
        client_status[i] = 0;
    }

    if ((socket_status = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        printf("Error socket.\n");
        return 0;
    }
    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(portNumber);
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Sinh IP ngẫu nhiên
    if (bind(socket_status, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Error bind.\n");
        return 0;
    }
    if ((listen(socket_status, 5) != 0))
    {
        printf("Error listen.\n");
        return 0;
    }
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(socket_status, &readfds);
        max_sd = socket_status;
        for (int i = 0; i < max_clients; i++)
        {
            int sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            printf("Error select.\n");
            return 0;
        }
        if (FD_ISSET(socket_status, &readfds))
        {
            if ((new_socket = accept(socket_status, (struct sockaddr *)&client, &slen)) < 0)
            {
                printf("Error accept.\n");
                return 0;
            }
            printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
            for (int i = 0; i < max_clients; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    client_status[i] = 0;
                    strcpy(client_username[i], "");
                    break;
                }
            }
        }
        for (int i = 0; i < max_clients; i++)
        {
            int connect_status = client_socket[i];
            if (FD_ISSET(connect_status, &readfds))
            {
                if (send_(client_socket, client_status, client_username, i, l, tree) == 0)
                {
                    Delete(tree, client_socket[i]);
                    WriteFileConnect(tree, "connect.txt");
                    close(connect_status);
                }
            }
        }
    }
    close(socket_status);
    return 0;
}
