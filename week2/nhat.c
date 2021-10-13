#include <stdio.h>  //printf
#include <string.h> //memset
#include <stdlib.h> //for exit(0);
#include <sys/socket.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <regex.h>
#define IP_OPTION 1
#define HOST_OPTION 2
#define WRONG 0
#define TRUE 1

int hostname_to_ip(char *);
void ip_to_host(char *);
void checkIpInCheckHostNameCase(char *hostname);
int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Wrong parameter\n");
        exit(1);
    }

    char valueConvert[50];
    strcpy(valueConvert, argv[1]);

    if (hostname_to_ip(valueConvert) == WRONG)
    {
        ip_to_host(valueConvert);
    }
    return 0;
}
/*
	Get ip from domain name
 */

int isValidDomain(char *domain)
{
    regex_t regex;
    int reti;
    reti = regcomp(&regex, "^(?!-)[A-Za-z0-9-]+([\\-\\.]{1}[a-z0-9]+)*\\.[A-Za-z]{2,6}$", REG_EXTENDED);
    if (reti)
    {
        return 1;
    }
    else
    {
        reti = regexec(&regex, domain, 0, NULL, 0);
        if (!reti)
        {
            return 1;
        }
        else
            return 0;
    }
}

void checkIpInCheckHostNameCase(char *hostname)
{
    if (isValidIpAddress(hostname))
    {
        printf("Not found information\n");
        exit(1);
    }
}
int hostname_to_ip(char *hostname)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i = 0;

    isValidDomain(hostname);

    if (((he = gethostbyname(hostname)) == NULL))
    {
        return WRONG;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_name));
    printf("Alias IP:\n");
    for (i = 0; addr_list[i] != NULL; i++)
    {
        printf("%s\n", inet_ntoa(*addr_list[i]));
    }
    return TRUE;
}

void ip_to_host(char *ipInput)
{
    typedef struct hostent DATA;

    typedef struct in_addr ADD;
    DATA *hostname;
    int i;

    if (!isValidIpAddress(ipInput))
    {
        printf("Not found information\n");
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