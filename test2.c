#include <stdio.h>
int main()
{
    FILE *fp;
    char str[] = "HocC!!!";

    fp = fopen("test2.txt", "r+");
    fwrite(str, 1, sizeof(str), fp);
    rewind(fp);
    char strTest[50];
    fscanf(fp, "%s", strTest);
    printf("%s\n", strTest);
    //     fscanf
    fclose(fp);

    return (0);
}