#include <stdio.h>
#include "stack.h"
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <regex.h>
#define IP_OPTION 1
#define HOST_OPTION 2
#define TRUE 1
#define WRONG 2
NODE *userLoged = NULL;
int isValidDomain(char *domain);
int isValidIpAddress(char *ipAddress);
void DNODE(DT x)
{
    printf("%-30s%-20s%-15d%-15d%-10s\n", x.userName, x.password, x.status, x.totalTimeWrongCode, x.homepageAddress);
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
void DLIST(LIST *l)
{
    NODE *p = l->Head;
    printf("%-35s%-20s%-15s%-30s%-20s\n", "Tên đăng nhập", "Mật khẩu", "Trạng thái", "Số lần đã nhập sai", "Homepage");
    while (p != NULL)
    {
        DNODE(p->x);
        p = p->next;
    }
}
void readFile(FILE *f, LIST *l)
{
    int lineCheck = 1;
    while (!feof(f))
    {
        DT x;
        x.totalTimeWrongCode = 0;
        x.totalTimeWrongPassword = 0;

        fscanf(f, "%s %s %d %s", x.userName, x.password, &x.status, x.homepageAddress);
        NODE *checkAccount = findAnAccount(l, x.userName);
        if (checkAccount != NULL)
        {
            printf("\n======================================\n");
            printf("Username input in line %d have existed. Check you file!\n", lineCheck);
            printf("======================================\n");
            exit(0);
        }
        if (!isValidDomain(x.homepageAddress) && !isValidIpAddress(x.homepageAddress))
        {
            printf("\n======================================\n");
            printf("Homepage address invalid in line %d\n", lineCheck);
            printf("======================================\n");
            exit(0);
        }
        Push(l, x);
        lineCheck++;
    }
    printf("\n\nYour file valid! Below is you data after insert:\n\n");
    DLIST(l);
}

void showMenu()
{
    printf("USER MANAGEMENT PROGRAM\n");
    printf("----------------------------------\n");
    printf("1. Register\n");
    printf("2. Active\n");
    printf("3. Sign in\n");
    printf("4. Search\n");
    printf("5.Change password\n");
    printf("6.Sign out\n");
    printf("7.Homepage with domain name\n");
    printf("8.Homepage with IP address\n");
    printf("Your choice (1-6, other to quit):");
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

        fwrite("\t", 1, 1, fptr);

        fwrite(p->x.homepageAddress, 1, strlen(p->x.homepageAddress), fptr);

        if (p->next != NULL)
        {
            fwrite("\n", 1, 1, fptr);
        }
        p = p->next;
    }
    fclose(fptr);
}

int isValidDomain(char *domain)
{
    struct hostent *he;
    if (((he = gethostbyname(domain)) == NULL))
    {
        return 0;
    }
    return 1;
}

void registerAccount(LIST *l, FILE *fptr)
{
    if (userLoged != NULL)
    {
        printf("Current user loged. Please log out to login another account!");
    }
    char userName[50];
    printf("Type you Username:");
    scanf("%[^\n]%*c", userName);
    if (strlen(userName) <= 1)
    {
        printf("Invalid username! Username must have length >=1");
        return;
    }
    NODE *check = findAnAccount(l, userName);
    if (check != NULL)
    {
        printf("Account existed!\n");
        return;
    }
    char password[50];
    char homepageAddress[50];
    printf("Type you Password:");
    scanf("%[^\n]%*c", password);
    printf("Type you homepage address:");
    scanf("%[^\n]%*c", homepageAddress);
    if (!isValidDomain(homepageAddress) && !isValidIpAddress(homepageAddress))
    {
        printf("Your homepage address invalid. So you could not register this account! Please try with another homepage address!");
        return;
    }
    DT x;
    strcpy(x.userName, userName);
    strcpy(x.password, password);
    strcpy(x.homepageAddress, homepageAddress);
    x.status = idle;
    x.totalTimeWrongCode = 0;
    x.totalTimeWrongPassword = 0;
    Push(l, x);
    printf("\n\nSuccessful registration. Activation required\n\n");
    writeFileAfterUpdate(l);
}
NODE *loginAccount(LIST *l)
{
    if (userLoged != NULL)
    {
        printf("Current user loged. Please log out to login another account!");
    }
    char userName[50];
    char password[50];
    printf("Please enter you account information to login\n");

    printf("UserName:");
    scanf("%[^\n]%*c", userName);

    NODE *user = findAnAccount(l, userName);
    if (user == NULL)
    {
        printf("Cannot find account\n");
        return NULL;
    }

    if (user->x.status == BLOCKED)
    {
        printf("Account is blocked\n\n");
        return NULL;
    }

    printf("Password:");
    scanf("%[^\n]%*c", password);

    if (user->x.status != ACTIVE)
    {
        printf("You have not permission to login this account.\n\n");
        return NULL;
    }

    while (strcmp(password, user->x.password) != 0 && user->x.totalTimeWrongCode != 3)
    {
        user->x.totalTimeWrongPassword++;
        if (user->x.totalTimeWrongPassword == 3)
        {
            printf("Password is incorrect. Account is blocked.\n\n");
            user->x.status = BLOCKED;
            writeFileAfterUpdate(l);

            return NULL;
            break;
        }
        printf("Password is incorrect. You have %d times left to enter ", 3 - user->x.totalTimeWrongPassword);

        printf("Password: ");
        scanf("%[^\n]%*c", password);
    }

    if (strcmp(user->x.password, password) == 0)
    {
        if (user->x.status != ACTIVE)
        {
            printf("You have not permission to login this account.\n");
            return NULL;
        }
        user->x.totalTimeWrongPassword = 0;
        printf("Hello %s\n\n\n", user->x.userName);
        return user;
    }

    return NULL;
}
void activeAccount(LIST *l)
{
    if (userLoged != NULL)
    {
        printf("Current user loged. Please log out to login another account!");
    }
    char userName[50];
    char password[50];
    int activeCode;

    printf("UserName:");
    scanf("%[^\n]%*c", userName);

    printf("Password:");
    scanf("%[^\n]%*c", password);

    NODE *user = findAnAccount(l, userName);
    if (user == NULL)
    {
        printf("This account not exist\n");
        return;
    }
    if (strcmp(user->x.password, password) != 0)
    {
        printf("Wrong Password!\n");
        return;
    }

    if (user->x.status != idle)
    {
        printf("You don't have permisison to use this.\n\n");
        return;
    }

    printf("Your active account code: ");
    scanf("%d%*c", &activeCode);

    while (activeCode != ACCTIVE_ACCOUNT_CODE && user->x.totalTimeWrongCode != 4)
    {
        user->x.totalTimeWrongCode++;
        if (user->x.totalTimeWrongCode == 4)
        {
            printf("Account is blocked.\n\n");
            user->x.status = BLOCKED;
            writeFileAfterUpdate(l);
            break;
        }
        printf("Account is not activated. You have %d times left to enter ", 4 - user->x.totalTimeWrongCode);

        printf("Your active account code: ");
        scanf("%d%*c", &activeCode);
    }

    if (activeCode == ACCTIVE_ACCOUNT_CODE)
    {
        user->x.status = ACTIVE;
        user->x.totalTimeWrongCode = 0;
        printf("Account is activated!\n\n");
    }

    return;
}

void search(LIST *l)
{
    if (userLoged == NULL)
    {
        printf("You don't have permission to do this.\n");
        return;
    }
    char userName[50];
    printf("Your username want to find:");
    scanf("%[^\n]%*c", userName);
    NODE *resultFind = findAnAccount(l, userName);
    if (resultFind == NULL)
    {
        printf("Cannot find account.");
        return;
    }
    printf("Your result:\n");

    switch (resultFind->x.status)
    {
    case BLOCKED:
        printf("Account is blocked\n");
        return;

    case ACTIVE:
        printf("Account is active\n");
        return;

    default:
        break;
    }

    return;
}

void changePassword()
{
    if (userLoged == NULL)
    {
        printf("You don't have permission to do this.\n\n");
        return;
    }
    char userName[50];
    printf("Type your user name:");
    scanf("%[^\n]%*c", userName);
    if (strcmp(userLoged->x.userName, userLoged->x.userName) != 0)
    {
        printf("Wrong username!");
        return;
    }

    char oldPassword[50];
    printf("Type your old password:");
    scanf("%[^\n]%*c", oldPassword);
    if (strcmp(oldPassword, userLoged->x.password) != 0)
    {
        printf("Current password is incorrect. Please try again!\n");
        return;
    }

    char newPassword[50];
    printf("Type your new password:");
    scanf("%[^\n]%*c", newPassword);
    strcpy(userLoged->x.password, newPassword);
    printf("Password is changed!\n");
    return;
}

void logOut(LIST *l)
{
    char userName[50];

    printf("Username:");
    scanf("%[^\n]%*c", userName);

    NODE *userWantLogout = findAnAccount(l, userName);
    if (userWantLogout == NULL)
    {
        printf("Cannot find account");
        return;
    }

    if (userLoged == NULL || strcmp(userWantLogout->x.userName, userLoged->x.userName) != 0)
    {
        printf("Account is not sign in\n\n");
        return;
    }

    printf("Good bye %s!\n", userLoged->x.userName);
    userLoged = NULL;
    return;
}

int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

void hostname_to_ip(char *hostname)
{
    if (userLoged == NULL)
    {
        printf("You don't have permission to use this controller.!");
        return;
    }
    struct hostent *he;
    struct in_addr **addr_list;
    int i = 0;

    isValidDomain(hostname);

    if (((he = gethostbyname(hostname)) == NULL))
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
    if (userLoged == NULL)
    {
        printf("You don't have permission to use this controller.!");
        return;
    }

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
void getDomainFromHomepage()
{
    if (userLoged == NULL)
    {
        printf("You dont have permission to use this controller!");
        return;
    }
    // In this case homepage in type ip address
    if (!isValidDomain(userLoged->x.homepageAddress))
    {
        ip_to_host(userLoged->x.homepageAddress);
        return;
    }
    // And this case home page is in type domain. So you just need print this domain.
    printf("The domain name: %s \n", userLoged->x.homepageAddress);
}
void getIpFromHomePage()
{

    if (userLoged == NULL)
    {
        printf("You dont have permission to use this controller!");
        return;
    }

    // In this case homepage now in type host - just domain name
    if (!isValidIpAddress(userLoged->x.homepageAddress))
    {
        hostname_to_ip(userLoged->x.homepageAddress);
        return;
    }
    // And this case home page is in type domain. So you just need print this domain.
    printf("The ip address: %s \n", userLoged->x.homepageAddress);
}
int main(int argc, char const *argv[])
{

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

    int n = 0;

    do
    {

        showMenu();
        scanf("%d%*c", &n);
        switch (n)
        {
        case 1:
            registerAccount(l, fp);
            printf("\n\n\n");
            break;
        case 2:
            activeAccount(l);
            break;
        case 3:
            userLoged = loginAccount(l);
            break;
        case 4:
            search(l);
            break;
        case 5:
            changePassword();
            break;
        case 6:
            logOut(l);
            break;
        case 7:
            getDomainFromHomepage();
            break;
        case 8:
            getIpFromHomePage();
            break;
        default:
            break;
        }
    } while (n > 0 && n < 9);

    Free(l);

    return 1;
}
