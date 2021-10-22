// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 1024
#define MAX_PORT 49151

int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

// Driver code
int main(int argc, char *argv[])
{
    printf("argc: %d\n", argc);
    if (argc < 3)
    {
        printf("Is not enought argument. Type input: ./client IPAddress {{PORT}}\n");
        exit(0);
    }
    // Validdate ip address

    char ipAddress[50];

    strcpy(ipAddress, argv[1]);
    if (!isValidIpAddress(ipAddress))
    {
        printf("Invalid IP address!");
        exit(0);
    }

    // Validate port
    int port = atoi(argv[2]);
    printf("port define: %d\n", port);

    if (port < 0 || port > 65535)
    {
        printf("Port number using must in range 0 -> 65535\n");
        return 0;
    }

    int sockfd;
    char receiveAfterUsername[MAXLINE];
    char bufferHandleData[MAXLINE];

    char *hello = "Hello from client";
    char *endConnect = "end";

    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    // set to IPv4
    servaddr.sin_family = AF_INET;

    servaddr.sin_port = htons(port);

    // store this IP address in sa:
    inet_pton(AF_INET, ipAddress, &(servaddr.sin_addr));

    int n, len, m;
    char *test = "";

    // Send data from server to client. Confirm we are connecting..
    sendto(sockfd, (const char *)test, strlen(test),
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           sizeof(servaddr));

    m = recvfrom(sockfd, (char *)bufferHandleData, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    bufferHandleData[m] = '\0';
    printf("Server : %s\n", bufferHandleData);

    char userName[50];
    char password[50];

    while (strcmp(receiveAfterUsername, endConnect) != 0)
    {

        printf("UserName:");
        scanf("%[^\n]%*c", userName);

        sendto(sockfd, (const char *)userName, strlen(userName),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));

        n = recvfrom(sockfd, (char *)receiveAfterUsername, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                     &len);
        receiveAfterUsername[n] = '\0';
        printf("%s\n", receiveAfterUsername);

        printf("Password:");
        scanf("%[^\n]%*c", password);

        sendto(sockfd, (const char *)password, strlen(password),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));
    }

    close(sockfd);
    return 0;
}
