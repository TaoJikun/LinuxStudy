/*
    将文件复制到分页程序
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 4096
#define DEF_PAGER "/bin/more" //适合屏幕查看的文件阅读输出工具。

int main(int argc, char *argv[])
{
    int n;
    int fd[2];
    pid_t pid;
    char *pager, *argv0;
    char line[MAXLINE];
    FILE *fp;

    if(argc != 2)
    {
        printf("usage: a.out <pathmae>\n");
        exit(1);
    }

    if((fp = fopen(argv[1], "r")) == NULL)
    {
        printf("can't open %s!\n", argv[1]);
        exit(1);
    }

    if(pipe(fd) < 0)
    {
        printf("failed to create pipe!\n");
        exit(1);
    }

    if((pid = fork()) < 0)
    {
        printf("faield to fork()!\n");
        exit(1);
    }
    else if(pid > 0)
    {
        close(fd[0]);//关闭父进程的读端
        //每次从fp中读取一行，然后写到管道中
        while((fgets(line, MAXLINE, fp)) != NULL)
        {
            n = strlen(line);
            if(write(fd[1], line, n) < 0)
            {
                printf("failed to write() to pipe!\n");
                exit(1);
            }
        }

        //检查fgets是否出错（用于区分到达文件结尾和出错两种情况）
        if(ferror(fp))
        {
            printf("fgets() error!\n");
            exit(1);
        }

        close(fd[1]);
        if(waitpid(pid, NULL, 0) < 0)
        {
            printf("failed to waitpid()!\n");
            exit(1);
        }
        exit(0);
    }
    else
    {
        close(fd[1]);//关闭子进程的写端
        if(fd[0] != STDIN_FILENO)
        {
            if(dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)//将fd[0]定向到标准输入
            {
                printf("failed to dup2()!\n");
                exit(1);
            }
            close(fd[0]);
        }

        //尝试使用环境变量PAGER试图获得用户分页程序名称。
        //如果这种操作没有成功，则使用系统默认值。
        //这是环境变量的常见用法。
        if((pager = getenv("PAGER")) == NULL)
        {
            //name为"PAGER"的环境变量不存在
            pager = DEF_PAGER;
        }

        /*  
            strrchr函数作用：
            用于查找一个字符c在另一个字符串str中末次出现的位置
            （也就是从str的右侧开始查找字符c首次出现的位置），
            并返回这个位置的地址。
            如果未能找到指定字符，那么函数将返回NULL。 
        */
        if((argv0 = strrchr(pager, '/')) != NULL)
        {
            argv0++;
        }
        else
        {
            argv0 = pager;
        }

        if(execl(pager, argv0, (char*)0) < 0)
        {
            printf("faield to execl for %s\n", pager);
        }
    }

    exit(0);
}