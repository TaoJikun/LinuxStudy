#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void* thread_fun1(void *arg);
void* thread_fun2(void *arg);

int main(void)
{
    int err;
    pthread_t tid1,tid2;
    void *tret;

    err = pthread_create(&tid1, NULL, thread_fun1, NULL);
    if(err != 0)
    {
        printf("%d can't create thread 1",err);
    }

    err = pthread_create(&tid2, NULL, thread_fun2, NULL);
    if(err != 0)
    {
        printf("%d can't create thread 2",err);
    }

    err = pthread_join(tid1,&tret);
    if(err != 0)
    {
        printf("%d can't join with thread 1",err);
    }
    printf("thread 1 exit code %ld \n", (long)tret);

    err = pthread_join(tid2,&tret);
    if(err != 0)
    {
        printf("%d can't join with thread 2",err);
    }
    printf("thread 2 exit code %ld \n", (long)tret);

    exit(0);
}

void* thread_fun1(void *arg)
{
    printf("thread 1 returnning\n");
    return ((void*)1);
}

void* thread_fun2(void *arg)
{
    printf("thread 2 returnning\n");
    return ((void*)2);
}