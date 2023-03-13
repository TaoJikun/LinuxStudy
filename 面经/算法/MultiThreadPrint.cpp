/*
    两个线程交替打印0~1，线程A打印偶数，线程B打印奇数
    两种方法实现：
    1. 互斥量+条件变量
    2. 互斥量+信号量
*/

#include <iostream>
#include <pthread.h>
#include <semaphore.h>

int i=0;
pthread_mutex_t mutex;
pthread_cond_t cond;
sem_t sem_odd;
sem_t sem_even;

/* 输出偶数 */
void* thread_cond_1(void*)
{
    while(i<=100)
    {
        pthread_mutex_lock(&mutex);
        
        //如果此时i不是偶数，则使用条件变量阻塞偶数进程，并释放锁
        //那么奇数进程就可以获得锁，并进行输出，然后再唤醒偶数进程
        if(i%2!=0){
            pthread_cond_wait(&cond,&mutex);
        }

        if(i<=100) printf("thread_cond_1: %d\n",i);
        i++;
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

/* 输出奇数 */
void* thread_cond_2(void*)
{
    while(i<=100)
    {
        pthread_mutex_lock(&mutex);
        
        if(i%2!=1){
            pthread_cond_wait(&cond,&mutex);
        }
        
        if(i<=100) printf("thread_cond_2: %d\n",i);
        i++;
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}


void* thread_sem_1(void*)
{
    while(i<=100)
    {

        sem_wait(&sem_even);
        if(i%2==0){
            if(i<=100) printf("thread_sem_1: %d\n",i);
            pthread_mutex_lock(&mutex);
            i++;
            pthread_mutex_unlock(&mutex);
        }
        sem_post(&sem_odd);
    }

    pthread_exit(0);
}


void* thread_sem_2(void*)
{
    while(i<=100)
    {

        sem_wait(&sem_odd);
        if(i%2==1){
            if(i<=100) printf("thread_sem_2: %d\n",i);
            pthread_mutex_lock(&mutex);
            i++;
            pthread_mutex_unlock(&mutex);
        }
        sem_post(&sem_even);
    }

    pthread_exit(0);
}


int main(int argc, char* argv[])
{
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
    
    sem_init(&sem_odd,0,0);
    sem_init(&sem_even,0,1);

    pthread_t thread1,thread2;
    pthread_create(&thread1,NULL,thread_sem_1,NULL);
    pthread_create(&thread2,NULL,thread_sem_2,NULL);
    
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    sem_destroy(&sem_odd);
    sem_destroy(&sem_even);

    return 0;
}