#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

#define MAXLINE 4096

jmp_buf jmpbuffer;

static void sig_alarm(int signo);

int main(void)
{
    int n;
    char line[MAXLINE];

    if (signal(SIGALRM,sig_alarm) == SIG_ERR)
    {
        printf("signal() error!\n");
        //exit(0);
    }

    if (setjmp(jmpbuffer) != 0)
    {
        //从别的函数返回
        printf("read timeout!\n");
        exit(0);
    }
    

    alarm(10);
    if((n = read(STDIN_FILENO,line,MAXLINE)) < 0)
    {
        printf("read() error!\n");
        //exit(0);
    }
    alarm(0);

    write(STDOUT_FILENO,line,n);

    exit(0);

}

void sig_alarm(int signo)
{
    longjmp(jmpbuffer,1);
}