#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


static void sig_cld(int signo);


/*
    此程序的问题是：在信号处理程序的开始处调用signal，内核检查是否有需要等待的子进程，
    因为我们正在处理一个SIGCLD信号，所以确实有这种子进程，所以它产生另一个对信号处理程序的调用。
    信号处理程序调用signal，整个过程再次重复。
    为了解决这一问题，应当在调用wait取到子进程的终止状态后再调用signal。
    此时仅当其他子进程终止，内核才会再次产生此种信号。

    ！当然了，我当前的系统中，#define SIGCLD SIGCHLD
*/

int main(void)
{
    pid_t pid;

    if(signal(SIGCLD,sig_cld) == SIG_ERR)
    {
        perror("signal() error!\n");
    }

    if((pid = fork()) < 0)
    {
        perror("fork() error!\n");
    }
    else if(pid == 0)
    {
        sleep(2);
        _exit(0);//产生SIGCLD信号
    }

    pause();
    exit(0);
}


void sig_cld(int signo)
{
    pid_t pid;
    int status;

    printf("SIGCLD recevied!\n");

    if(signal(SIGCLD,sig_cld) == SIG_ERR)
    {
        perror("signal() error!\n");
    }

    if((pid = wait(&status)) < 0)
    {
        perror("wait() error!\n");
    }

    printf("pid = %d \n",pid);
}