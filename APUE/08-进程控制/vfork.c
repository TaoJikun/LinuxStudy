#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int globval = 6;

int main(void)
{
    int var;
    __pid_t pid;

    var =88;
    printf("before fork()\n");

    if ((pid = vfork()) < 0)
    {
        printf("vfork() error!\n");
    }

    if (pid == 0)
    {
        /* child */
        globval++;
        var++;
        printf("pid = %ld, globval = %d, var = %d \n",(long)getpid(),globval,var);
        printf("ppid() = %ld \n",(long)getppid());
        fclose(stdout);
        _exit(0);//子进程执行了exit函数后父进程才会被调度运行
    }

    printf("pid = %ld, globval = %d, var = %d \n",(long)getpid(),globval,var);

    exit(0);
}