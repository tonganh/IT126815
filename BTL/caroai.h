#include <stdio.h>
#include <string.h>

int size, player, cpu;
char *table;

int max(int a, int b);

int min(int a, int b);

void setTable(char* t, int s, int p, int c);

int isNullCell(int col, int row);

int setCEll(int col, int row, int value);

int getCell(int col, int row);

int getRank(int col, int row);

int isWin(int col, int row, int player);

int playerMove(int col, int row);

int cpuMove(int *col, int *row);