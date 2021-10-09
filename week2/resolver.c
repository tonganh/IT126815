#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //for exit(0);
#include <sys/socket.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <arpa/inet.h>
#define IP_OPTION 1
#define HOST_OPTION 2
void hostname_to_ip(char *);
void ip_to_host(char *);
int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Your parameter invalid.\n");
        exit(1);
    }

    int optionChoose = atoi(argv[1]);
    char valueConvert[50];
    strcpy(valueConvert, argv[2]);
    switch (optionChoose)
    {

    case IP_OPTION:
        ip_to_host(valueConvert);
        break;
    case HOST_OPTION:
        hostname_to_ip(valueConvert);
        break;

    default:
        break;
    }
    return 0;
}
/*
	Get ip from domain name
 */

void hostname_to_ip(char *hostname)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i = 0;

    if ((he = gethostbyname(hostname)) == NULL)
    {
        // get the host info
        printf("Not found information\n");
        exit(1);
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_name));
    printf("Alias IP:\n");
    for (i = 0; addr_list[i] != NULL; i++)
    {
        printf("%s\n", inet_ntoa(*addr_list[i]));
    }
}

void ip_to_host(char *ipInput)
{
    typedef struct hostent DATA;
    typedef struct in_addr ADD;
    DATA *hostname;
    int i;

    if (!isValidIpAddress(ipInput))
    {
        printf("Wrong parameter\n");
        return;
    }
    ADD ip;
    inet_aton(ipInput, &ip);
    hostname = gethostbyaddr(&ip, sizeof(ip), AF_INET);
    if (hostname)
    {
        printf("Official name: %s\n", hostname->h_name);
        printf("Alias name:\n");
        i = 0;
        while (hostname->h_aliases[i] != NULL)
        {
            printf("%s\n", hostname->h_aliases[i++]);
        }
    }
    else
    {
        printf("Not found information\n");
    }
    return;
}