#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct foo{
    int a,b,c,d;
};


void printfoo(char *s , const struct foo *fp);

void* thread_fun1(void *arg);
void* thread_fun2(void *arg);


int main(void)
{
    int err;
    pthread_t tid1,tid2;
    struct foo *fp;

    err = pthread_create(&tid1, NULL, thread_fun1, NULL);
    if(err != 0)
    {
        printf("%d can't create thread 1",err);
    }

    err = pthread_join(tid1,(void*)&fp);//阻塞等待线程tid1终止，并获取其返回码
    if(err != 0)
    {
        printf("%d can't join with thread 1",err);
    }
    sleep(1);

    printf("parent starting second thread\n");

    err = pthread_create(&tid2, NULL, thread_fun2, NULL);
    if(err != 0)
    {
        printf("%d can't create thread 2",err);
    }
    sleep(1);

    printfoo("parent: \n",fp);

    exit(0);
}

void printfoo(char *s , const struct foo *fp)
{
    printf("%s",s);
    printf(" struct foo at 0x%lx \n",(unsigned long)fp);
    printf(" foo.a = %d\n",fp->a);
    printf(" foo.b = %d\n",fp->b);
    printf(" foo.c = %d\n",fp->c);
    printf(" foo.d = %d\n",fp->d);
}

void* thread_fun1(void *arg)
{
    struct foo fp = {1,2,3,4};
    printfoo("thread 1 :\n",&fp);
    pthread_exit((void*)&fp);//返回码为局部变量，主线程调用pthread_join后获得的返回码结构体已经被析构
}

void* thread_fun2(void *arg)
{
    printf("thread 2 : ID is %lu\n", (unsigned long)pthread_self());
    pthread_exit((void*)0);
}