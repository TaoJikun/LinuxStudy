#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

#define MAXLINE 4096

static void sig_sigpipe(int signo)
{
    if(signo == SIGPIPE)
    {
        printf("SIGPIPE caught!\n");
        exit(1);
    }
}

int main(void)
{
    int n, fd1[2], fd2[2];
    pid_t pid;
    char line[MAXLINE];

    if(signal(SIGPIPE,sig_sigpipe) < 0)
    {
        printf("signal() error!\n");
        exit(1);
    }

    if(pipe(fd1) < 0 || pipe(fd2) < 0)
    {
        printf("pipe() error!\n");
        exit(1);
    }

    if((pid = fork()) < 0)
    {
        printf("fork() error!\n");
        exit(1);
    }
    else if(pid == 0)
    {
        close(fd1[1]);
        close(fd2[0]);
        
        //将管道重定向到标准I/O上可以等效看作：
        //将fd1[0]读到的数据作为标准输入
        //将标准输出的数据写进fd2[1]
        if(fd1[0] != STDIN_FILENO)
        {
            if(dup2(fd1[0],STDIN_FILENO) != STDIN_FILENO)
            {
                printf("dup2() error to stdin!\n");
                exit(1);
            }
            close(fd1[0]);
        }

        if(fd2[1] != STDOUT_FILENO)
        {
            if(dup2(fd2[1],STDOUT_FILENO) != STDOUT_FILENO)
            {
                printf("dup2() error to stdout!\n");
                exit(1);
                close(fd2[1]);
            }
        }

        if(execl("./add", "add", (char*)0) < 0)
        {
            printf("execl() error!\n");
            exit(1);
        }

        exit(0);
    }
    else
    {
        close(fd1[0]);
        close(fd2[1]);

        while(fgets(line, MAXLINE, stdin) != NULL)
        {
            n = strlen(line);
            if(write(fd1[1], line, n) != n)
            {
                printf("write() error!\n");
                exit(1);
            }
            if((n = read(fd2[0], line, MAXLINE)) < 0)
            {
                printf("read() error!\n");
                exit(1);
            }
            if(n==0)
            {
                printf("child close pipe fd2!\n");
                break;
            }
            line[n]=0;
            if(fputs(line, stdout) == EOF)
            {
                printf("fputs error!\n");
                exit(1);
            }
        }

        if(ferror(stdin))
        {
            printf("fgets() error!\n");
            exit(1);
        }
        exit(0);
    }

    exit(0);
}