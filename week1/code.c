#include <stdio.h>
#include "stack.h"
#include <string.h>

NODE *userLoged = NULL;
void DNODE(DT x)
{
    printf("%-30s%-20s%-15d%-10d\n", x.userName, x.password, x.status, x.totalTimeWrongCode);
}
// NODE* findNodeWithUserName(LIST* l){
//     NODE*  p = l->Head;
// }
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
    printf("%-35s%-20s%-15s%-10s\n", "Tên đăng nhập", "Mật khẩu", "Trạng thái", "Số lần đã nhập sai");
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
    // DLIST(l);
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
        if (p->next != NULL)
        {
            fwrite("\n", 1, 1, fptr);
        }
        p = p->next;
    }
    fclose(fptr);
}

void registerAccount(LIST *l, FILE *fptr)
{
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
    printf("Type you Password:");
    scanf("%[^\n]%*c", password);
    DT x;
    strcpy(x.userName, userName);
    strcpy(x.password, password);
    x.status = idle;
    x.totalTimeWrongCode = 0;
    x.totalTimeWrongPassword = 0;
    Push(l, x);
    printf("\n\nSuccessful registration. Activation required\n\n");
    writeFileAfterUpdate(l);
}
NODE *loginAccount(LIST *l)
{
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
        default:
            break;
        }
    } while (n > 0 && n < 7);

    Free(l);

    return 1;
}
