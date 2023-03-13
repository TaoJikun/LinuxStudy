/*
    以分离状态创建线程
*/
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


int make_thread_detach(void* (*func)(void*), void* arg)
{
    int err;
    pthread_t tid;
    pthread_attr_t attr;

    err = pthread_attr_init(&attr);
    if(err != 0)
    {
        return err;
    }

    err = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    if(err == 0)
    {
        err = pthread_create(tid, &attr, func, arg);
    }

    pthread_attr_destroy(&attr);
    return err;
}
