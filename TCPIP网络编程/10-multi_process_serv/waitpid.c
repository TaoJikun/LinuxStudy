#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int status;
    __pid_t pid=fork();

    if(pid==0){
        sleep(15);
        return 24;
    }
    else{
        printf("Child PID: %d \n",pid);
        while(!(pid = waitpid(-1,&status,WNOHANG)))
        {
            printf("waitpid return pid %d ,sleep 3 sec.\n", pid);
            sleep(3);
        }

        if(WIFEXITED(status)){
            printf("waitpid return pid %d \n", pid);
            printf("Child send %d \n", WEXITSTATUS(status));
        }
    }

    return 0;
}