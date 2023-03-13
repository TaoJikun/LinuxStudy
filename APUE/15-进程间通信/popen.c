/*
    使用popen函数将文件复制到分页程序
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
    shell命令${PAGER:-more}的意思是：
    如果shell变量PAGER已经定义，且其值非空，则使用其值，
    否则使用字符串more。
*/
#define PAGER "${PAGER:-more}"
#define MAXLINE 4096

int main(int argc, char *argv[])
{
    char line[MAXLINE];
    FILE *fpin, *fpout;

    if(argc != 2)
    {
        printf("usage: a.out <pathname>!\n");
        exit(1);
    }

    //popen，读argv[1]指定的文件的内容
    if((fpin = popen(argv[1], "r")) == NULL)
    {
        printf("can't open %s!\n", argv[1]);
        exit(1);
    }

    //popen，向分页程序写
    if((fpout = popen(PAGER, "w")) == NULL)
    {
        printf("popen() error!\n");
        exit(1);
    }

    while(fgets(line, MAXLINE, fpin) != NULL)
    {
        if(fputs(line, fpout) == EOF)
        {
            printf("fputs error to pipe!\n");
            exit(1);
        }
    }

    if(ferror(fpin))
    {
        printf("fegts() error!\n");
        exit(1);
    }

    if(pclose(fpout) == -1)
    {
        printf("pclose failed!\n");
        exit(1);
    }

    exit(0);
}