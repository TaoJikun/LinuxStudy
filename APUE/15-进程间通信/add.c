/*
    两数相加的简单过滤程序
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 4096

int main(void)
{
    int n, int1, int2;
    char line[MAXLINE];

    while((n = read(STDIN_FILENO, line, MAXLINE)) > 0)
    {
        line[n] = 0;
        if(sscanf(line, "%d%d", &int1, &int2) == 2)//将line中的数值写入int1、int2
        {
            sprintf(line, "%d\n", int1+int2);
            n = strlen(line);
            if(write(STDOUT_FILENO, line, n) != n)
            {
                printf("write() error!\n");
                exit(1);
            }
        }
        else
        {
            if(write(STDOUT_FILENO, "invalid args\n", 13) != 13)
            {
                printf("write() error!\n");
                exit(1);
            }
        }
    }

}