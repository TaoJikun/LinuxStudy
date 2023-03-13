#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

pthread_t ntid;


void printids(const char *s);

void* thread_fun(void *arg);


int main(void)
{
    int err;
    err = pthread_create(&ntid, NULL, thread_fun, NULL);//主线程创建一个线程，新线程调用自定义打印函数
    if (err != 0)
    {
        printf("%d can't create thread!\n", err);
    }
    printids("main thread: ");

    sleep(1);//主线程需要休眠，如果主线程不休眠，它就可能退出，这样在新线程有机会运行之前整个进程可能就已经终止了
    exit(0);
    
}

void printids(const char *s)
{
    pid_t pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();//获取调用线程的线程ID

    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid,
        (unsigned long)tid, (unsigned long)tid);
}

void* thread_fun(void *arg)
{
    printids("new thread: ");
    return((void*)0);
}