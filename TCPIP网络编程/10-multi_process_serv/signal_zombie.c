#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void read_childproc(int signo)
{
    int status;
    pid_t pid;
    pid = waitpid(-1,&status,WNOHANG);
    if(WIFEXITED(status)){
            printf("Remove proc pid: %d \n", pid);
            printf("Child send %d \n", WEXITSTATUS(status));
    }

    /*while((pid = waitpid(-1,&status,WNOHANG)) > 0)
    {
        if(WIFEXITED(status)){
            printf("Remove proc pid: %d \n", pid);
            printf("Child send %d \n", WEXITSTATUS(status));
        }
    }*/
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler=read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGCHLD,&act,0);
    

    __pid_t pid=fork();

    if(pid==0){
        puts("Hi!I am Child Process1!");
        sleep(10);
        return 12;
    }
    else{
        printf("Child PID: %d \n",pid);
        pid = fork();
        if ((pid==0)){
            puts("Hi,I am child process2!");
            sleep(15);
            exit(24);
        }
        else{
            int i;
            printf("Child PID: %d \n",pid);
            for(i=0;i<5;++i)
            {
                puts("wait...");
                sleep(5);
            }
        }
    }

    return 0;
}