#include <stdio.h>
#include <unistd.h>

int gval = 10;
int main(int argc, char *argv[])
{
    __pid_t pid;
    int lval = 20;
    gval++;
    lval+=5;

    pid = fork();
    if(pid==0){
        //该进程是子进程
        gval+=2;
        lval+=2;
    }
    else{
        //该进程是父进程
        gval-=2;
        lval-=2;
    }

    if(pid==0)
        printf("Child Proc: [%d, %d] \n",gval,lval);
    else
        printf("Parent Proc: [%d, %d] \n",gval,lval);

    return 0;
}