#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int globval = 6;
char buf[] = "a write to stdout!\n";


int main(void)
{
    int var;
    __pid_t pid;

    var =88;
    if (write(STDOUT_FILENO,buf,sizeof(buf)-1) != sizeof(buf)-1)
    {
        printf("write() error!\n");
    }
    printf("before fork()\n");

    if ((pid = fork()) < 0)
    {
        printf("fork() error!\n");
    }

    if (pid == 0)
    {
        /* child */
        globval++;
        var++;
    }
    else
    {
        /* parent */
        sleep(2);
    }

    printf("pid = %ld, globval = %d, var = %d \n",(long)getpid(),globval,var);

    exit(0);
}