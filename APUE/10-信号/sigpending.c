#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static void sig_quit(int signo);

int main(void)
{
    sigset_t newmask, oldmask, pendmask;

    if(signal(SIGQUIT,sig_quit) == SIG_ERR)//注册SIGQUIT的信号处理函数
    {
        printf("signal() error!\n");
    }

    sigemptyset(&newmask);//将newmask置为空
    sigaddset(&newmask,SIGQUIT);//将SIGQUIT添加到newmask中
    
    if(sigprocmask(SIG_BLOCK,&newmask,&oldmask) < 0)//将当前信号屏蔽字和newmask中的信号屏蔽字求并集
    {
        printf("sigprocmask() error!\n");
    }

    sleep(5); //此时在终端中键入ctrl+\，产生SIGQUIT信号，但是此时该信号被阻塞

    if(sigpending(&pendmask) < 0)//获取在送往进程的时候被阻塞挂起的信号集合
    {
        printf("\nsigpending() error!\n");
    }

    if(sigismember(&pendmask,SIGQUIT))//判断SIGQUIT信号是否在送往进程是会被阻塞
    {
        printf("SIGQUIT pending!\n");
    }
    
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)//将信号屏蔽字还原，这时SIG_QUIT被解除阻塞，将前面产生的SIGQUIT信号递送给该进程
    {
        printf("SIG_SETMASK error!\n");
    }
    printf("SIGQUIT unblocked\n");
    
    sleep(5);
    exit(0);



}

void sig_quit(int signo)
{
    printf("Caught SIGQUIT!\n");
    if(signal(SIGQUIT,SIG_DFL) == SIG_ERR)
    {
        printf("can't reset SIGQUIT!\n");
    }
}