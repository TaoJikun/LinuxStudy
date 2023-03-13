#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void* thread_main(void* argv);

int main(int argc, char *argv[])
{
    pthread_t thread_id;
    int thread_param = 5;
    void* thread_ret;

    if(pthread_create(&thread_id, NULL, thread_main, (void*)&thread_param)!=0)
    {
        puts("pthread_cread() error!");
        return -1;
    }
    
    if(pthread_join(thread_id,&thread_ret) != 0)
    {
        puts("pthread_join() error!");
        return -1;
    }

    printf("Thread Return Message: %s \n", (char*)thread_ret);
    free(thread_ret);
    puts("end of main!");
    return 0;
}


void* thread_main(void* argv)
{
    int i;
    int cnt = *((int*)argv);
    char * msg = (char*)malloc(sizeof(char)*50);
    strcpy(msg,"Hello, I'am thread~ \n");

    for(i=0;i<cnt;++i)
    {
        sleep(1);
        puts("running thread!");
    }
    return (void*)msg;
}