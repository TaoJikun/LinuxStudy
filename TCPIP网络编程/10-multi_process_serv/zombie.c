#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    __pid_t pid = fork();
    
    if(pid==0){
        //该进程是子进程
        puts("Hi, I am a child process");
    }
    else{
        //该进程是父进程
        printf("Child Process ID: %d \n",pid);
        sleep(30);//休眠30秒
    }

    if(pid==0)
        puts("End child process");
    else
        puts("End parent process");

    return 0;
}