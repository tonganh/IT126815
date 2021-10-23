// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "stack.h"
#define MAXLINE 1024
#define MAX_PORT 49151

void getInput(char *input)
{
    int i = 0;
    char c = getchar();
    while (c != '\n')
    {
        input[i++] = c;
        c = getchar();
    }
    input[i] = '\0';
    fflush(stdin);
}

int convertValue(char *value)
{
    if (strcmp(value, "1") == 0)
    {
        printf("OK\n");
        return ACTIVE;
    }

    if (strcmp(value, "0") == 0 || strcmp(value, "3") == 0)
    {
        printf("Account not ready\n");
        return BLOCKED;
    }
    printf("Not OK\n");
    return WRONG_PASSWORD;
}
int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

// Driver code
int main(int argc, char *argv[])
{
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
    printf("Connecting to %s in port %d\n", ipAddress, port);

    if (port < 0 || port > 65535)
    {
        printf("Port number using must in range 0 -> 65535\n");
        return 0;
    }

    int sockfd;
    char receiveAfterUsername[MAXLINE];
    char bufferHandleData[MAXLINE];

    char *endConnect = "end";

    struct sockaddr_in servaddr;

    fflush(stdin);

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

    int n, m;
    socklen_t len;
    char *test = "";

    // Send data from server to client. Confirm we are connecting..
    sendto(sockfd, (char *)test, strlen(test),
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
        getInput(userName);

        fflush(stdin);
        sendto(sockfd, (char *)userName, strlen(userName),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));
        //    Turn off
        if (strcmp(userName, "") == 0)
        {
            printf("\nBye\n");
            break;
        }
        n = recvfrom(sockfd, (char *)receiveAfterUsername, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                     &len);

        printf("Password:");
        getInput(password);
        fflush(stdin);

        sendto(sockfd, (char *)password, strlen(password),
               MSG_CONFIRM, (struct sockaddr *)&servaddr,
               sizeof(servaddr));

        char test[50];
        n = recvfrom(sockfd, (char *)test, sizeof(test),
                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                     &len);
        test[n] = '\0';

        // Now going to change password controller
        if (convertValue(test) == ACTIVE)
        {
            char changePassword[100];
            memset(changePassword, '\0', sizeof(changePassword));
            getInput(changePassword);
            fflush(stdin);

            if (strcmp(changePassword, "") == 0)
            {
                printf("1.End.\n");
                sendto(sockfd, (char *)changePassword, strlen(changePassword),
                       MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                       sizeof(struct sockaddr));

                break;
            }

            while (strcmp(changePassword, "") != 1)
            {
                sendto(sockfd, (char *)changePassword, strlen(changePassword),
                       MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                       sizeof(struct sockaddr));

                if (strcmp("bye", changePassword) == 0)
                {
                    printf("Goodbye %s\n", userName);
                    break;
                }

                char dataAfterChangePW[50];

                n = recvfrom(sockfd, (char *)dataAfterChangePW, sizeof(dataAfterChangePW),
                             MSG_WAITALL, (struct sockaddr *)&servaddr,
                             &len);
                if (strcmp(dataAfterChangePW, "Fail") == 0)
                {
                    printf("Error\n");
                }
                else
                {
                    char passwordAlpha[50];
                    char passwordDigit[50];

                    // memset for variable
                    memset(passwordAlpha, '\0', sizeof(passwordAlpha));
                    memset(passwordDigit, '\0', sizeof(passwordDigit));

                    // Handle data:
                    n = recvfrom(sockfd, (char *)passwordAlpha, sizeof(passwordAlpha),
                                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                                 &len);
                    printf("%s\n", passwordAlpha); //In mỗi token ra

                    n = recvfrom(sockfd, (char *)passwordDigit, sizeof(passwordDigit),
                                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                                 &len);

                    printf("%s\n", passwordDigit); //In mỗi token ra

                    // Reset
                    memset(passwordAlpha, 0, sizeof(passwordAlpha));
                    memset(passwordDigit, 0, sizeof(passwordDigit));

                    // printf("passwordAfterEncode: %s\n", dataAfterChangePW);
                    // char *token = strtok(dataAfterChangePW, ",");
                    // while (token != NULL)
                    // {
                    //     printf("After split: %s\n", token); //In mỗi token ra
                    //     token = strtok(NULL, " ");
                    // }
                }
                // Reset string
                memset(&changePassword, 0, sizeof(changePassword));
                memset(&dataAfterChangePW, 0, sizeof(dataAfterChangePW));
                scanf("%[^\n]%*c", changePassword);
                if (strcmp(changePassword, "") == 0)
                {
                    sendto(sockfd, (char *)changePassword, strlen(changePassword),
                           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                           sizeof(struct sockaddr));
                    printf("End.\n");
                    break;
                }
            }
        }
    }
    close(sockfd);
    return 0;
}
