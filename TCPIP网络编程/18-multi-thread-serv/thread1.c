#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* thread_main(void* argv);

int main(int argc, char *argv[])
{
    pthread_t thread_id;
    int thread_param = 5;

    if(pthread_create(&thread_id, NULL, thread_main, (void*)&thread_param)!=0)
    {
        puts("pthread_cread() error!");
        return -1;
    }
    printf("thread id: %ld \n",thread_id);
    sleep(10);
    puts("end of main!");
    return 0;
}


void* thread_main(void* argv)
{
    int i;
    int cnt = *((int*)argv);
    for(i=0;i<cnt;++i)
    {
        sleep(1);
        puts("running thread!");
    }
    return NULL;
}