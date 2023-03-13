#include <stdio.h>
#include <pthread.h>

void* thread_sumation(void* argv);
int sum=0;

int main(int argc, char *argv[])
{
    pthread_t thread_id1,thread_id2;
    int range1[] = {1,5};
    int range2[] = {6,10};

    pthread_create(&thread_id1,NULL,thread_sumation,(void*)&range1);
    pthread_create(&thread_id2,NULL,thread_sumation,(void*)&range2);

    pthread_join(thread_id1,NULL);
    pthread_join(thread_id2,NULL);

    printf("result: %d \n",sum);

    return 0;
}


void* thread_sumation(void* argv)
{
    int start = ((int*)argv)[0];
    int end = ((int*)argv)[1];
    while(start <= end)
    {
        sum+=start;
        start++;
    }

    return NULL;
}