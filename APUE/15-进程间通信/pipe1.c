#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define MAXLINE 4096

int main(void)
{
    int readn;
    char line[MAXLINE];
    int fd[2];
    pid_t pid;

    if(pipe(fd) < 0)
    {
        printf("failed to create pipe!\n");
        exit(1);
    }

    if((pid = fork()) < 0)
    {
        printf("failed to fork()!\n");
        exit(1);
    }
    else if(pid > 0)
    {
        close(fd[0]);//关闭父进程的读端
        write(fd[1], "hello world!\n", 13);
    }
    else
    {
        close(fd[1]);//关闭子进程的写端
        if((readn = read(fd[0], line, MAXLINE)) < 0)
        {
            printf("failed to read!\n");
            exit(1);
        }
        write(STDOUT_FILENO, line, readn);
    }

    exit(0);
}
