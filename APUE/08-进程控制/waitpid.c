#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


int main()
{
    __pid_t pid;

    if((pid = fork()) < 0)
    {
        printf("fork() error!\n");
    }
    else if(pid == 0)
    {
        if((pid = fork()) < 0)
        {
            printf("fork() error!\n");
        }
        else if(pid > 0)
        {
            exit(0);
        }

        sleep(2);
        printf("second child, parent pid = %ld \n",(long)getppid());
        exit(0);
    }

    if (waitpid(pid,NULL,0) != pid)
    {
        printf("waitpid() error!\n");
    }

    exit(0);
    
}
