/*
    三个线程交替打印A、B、C，线程1打印A，线程2打印B，线程3打印C
    两种方法实现：
    1. 互斥量+条件变量
    2. 互斥量+三个信号量
*/

#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

int i;
pthread_mutex_t mutex;
pthread_cond_t cond;

sem_t semA;
sem_t semB;
sem_t semC;



void* printA_cond(void*)
{
    while(i<30){
        pthread_mutex_lock(&mutex);

        if(i%3 != 0){
            pthread_cond_wait(&cond, &mutex);
        }

        if(i<30 && i%3 == 0) {
            cout<<"A---"<<i<<endl;
            i++;
        }

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}


void* printB_cond(void*)
{
    while(i<30){
        pthread_mutex_lock(&mutex);

        if(i%3 != 1){
            pthread_cond_wait(&cond, &mutex);
        }

        if(i<30 && i%3 == 1) {
            cout<<"B---"<<i<<endl;
            i++;
        }

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}


void* printC_cond(void*)
{
    while(i<30){
        pthread_mutex_lock(&mutex);

        if(i<30 && i%3 != 2){
            pthread_cond_wait(&cond, &mutex);
        }

        if(i%3 == 2) {
            cout<<"C---"<<i<<endl;
            i++;
        }

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}




void* printA_sem(void*)
{
    while(i<30)
    {
        sem_wait(&semA);
        if(i<30 && i%3==0){
            cout<<"A----"<<i<<endl;
            pthread_mutex_lock(&mutex);
            i++;
            pthread_mutex_unlock(&mutex);
        }
        sem_post(&semB);
    }

    pthread_exit(0);
}


void* printB_sem(void*)
{
    while(i<30)
    {
        sem_wait(&semB);
        if(i<30 && i%3==1){
            cout<<"B----"<<i<<endl;
            pthread_mutex_lock(&mutex);
            i++;
            pthread_mutex_unlock(&mutex);
        }
        sem_post(&semC);
    }

    pthread_exit(0);
}


void* printC_sem(void*)
{
    while(i<30)
    {
        sem_wait(&semC);
        if(i<30 && i%3==2){
            cout<<"C----"<<i<<endl;
            pthread_mutex_lock(&mutex);
            i++;
            pthread_mutex_unlock(&mutex);
        }
        sem_post(&semA);
    }

    pthread_exit(0);
}


int main()
{
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);

    sem_init(&semA,0,1);
    sem_init(&semB,0,0);
    sem_init(&semC,0,0);

    i=0;

    pthread_t thread1,thread2,thread3;
    
    pthread_create(&thread1,NULL,printA_sem,NULL);
    pthread_create(&thread2,NULL,printB_sem,NULL);
    pthread_create(&thread3,NULL,printC_sem,NULL);

    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    pthread_join(thread3,NULL);

    exit(0);
}