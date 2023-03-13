#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


void print_exit(int status);

int main(void)
{
    __pid_t pid;
    int status;

    if ((pid = fork()) < 0)
    {
        printf("fork() error!\n");
    }
    else if(pid==0)
    {
        exit(7);//子进程正常退出
    }

    if(wait(&status) != pid)
    {
        printf("wait error!\n");
    }
    print_exit(status);


    if ((pid = fork()) < 0)
    {
        printf("fork() error!\n");
    }
    else if(pid==0)
    {
        status  = status/0;//一个数除以0，产生信号SIGFPE
    }

    if(wait(&status) != pid)
    {
        printf("wait error!\n");
    }
    print_exit(status);


    if ((pid =fork() < 0))
    {
        printf("fork() error!\n");
    }
    else if(pid == 0)
    {
        abort();//子进程异常终止，产生信号SIGABRT
    }

    if(wait(&status) != pid)
    {
        printf("wait() error!\n");
    }
    print_exit(status);
    

    exit(0);
    
}


void print_exit(int status)
{
    if (WIFEXITED(status))
    {
        printf("normal termination, exit status = %d \n", WEXITSTATUS(status));
    }
    else if(WIFSIGNALED(status))
    {
        printf("abnormal termination, signal number = %d%s \n", 
            WTERMSIG(status),
#ifdef WCOREDUMP
            WCOREDUMP(status)?"(core file generated)":"");
#else
            "");
#endif
    }
    else if (WIFSTOPPED(status))
    {
        printf("child stopped, signal number = %d \n",WSTOPSIG(status));
    }
}