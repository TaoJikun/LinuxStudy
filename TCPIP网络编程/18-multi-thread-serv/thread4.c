#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREAD 100

void* thread_inc(void *argv);
void* thread_dec(void *argv);

long long num=0;

int main(int argc, int *argv[])
{
    pthread_t thread_id[NUM_THREAD];

    for (int i = 0; i < NUM_THREAD; i++)
    {
        if(i%2)
            pthread_create(&(thread_id[i]),NULL,thread_inc,NULL);
        else
            pthread_create(&(thread_id[i]),NULL,thread_dec,NULL);
    }

    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(thread_id[i],NULL);
    }

    printf("result: %ld \n",num);    

    return 0;
}


void* thread_inc(void* argv)
{
    for(int i=0;i<50000000;i++)
        num++;
    return NULL;
}

void* thread_dec(void* argv)
{
    for(int i=0;i<50000000;i++)
        num--;
    return NULL;
}