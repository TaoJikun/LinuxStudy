#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static void sig_usr(int sig);


int main(void)
{
    if(signal(SIGUSR1,sig_usr) == SIG_ERR)
    {
        perror("can't catch SIGUSR1!\n");
    }
    if(signal(SIGUSR2,sig_usr) == SIG_ERR)
    {
        perror("can't catch SIGUSR2!\n");
    }
    for(;;){
        pause();
    }
}


static void sig_usr(int sig)
{
    if (sig == SIGUSR1)
    {
        printf("received SIGUSR1!\n");
    }
    else if(sig == SIGUSR2)
    {
        printf("received SIGUSR2!\n");
    }
    else
    {
        printf("recevied signal %d",sig);
    }
    
}
