#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void cleanup(void *arg);
void* thread_fun1(void *arg);
void* thread_fun2(void *arg);

int main(void)
{
    int err;
    pthread_t tid1,tid2;
    void *tret;

    err = pthread_create(&tid1,NULL,thread_fun1,(void*)1);
    if (err != 0)
    {
        printf("%d can't create thread 1\n",err);
    }
    sleep(1);
    
    err = pthread_create(&tid2,NULL,thread_fun2,(void*)2);
    if (err != 0)
    {
        printf("%d can't create thread 2\n",err);
    }
    sleep(1);
    
    err = pthread_join(tid1,&tret);
    if(err!=0)
    {
        printf("%d can't join thread 1\n",err);
    }
    printf("thread 1 exit code %ld\n",(long)tret);

    err = pthread_join(tid2,&tret);
    if(err!=0)
    {
        printf("%d can't join thread 2\n",err);
    }
    printf("thread 2 exit code %ld\n",(long)tret);

    exit(0);
}

void cleanup(void *arg)
{
    printf("cleanup: %s\n",(char*)arg);
}

void* thread_fun1(void *arg)
{
    printf("thread 1 start\n");
    pthread_cleanup_push(cleanup,"thread 1 first handler");
    pthread_cleanup_push(cleanup,"thread 1 second handler");
    printf("thread 1 push complete\n");
    if (arg)
    {
        return ((void*)1);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return ((void*)1);
}

void* thread_fun2(void *arg)
{
    printf("thread 2 start\n");
    pthread_cleanup_push(cleanup,"thread 2 first handler");
    pthread_cleanup_push(cleanup,"thread 2 second handler");
    printf("thread 2 push complete\n");
    if (arg)
    {
        pthread_exit((void*)2);
    }
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_exit((void*)2);
}