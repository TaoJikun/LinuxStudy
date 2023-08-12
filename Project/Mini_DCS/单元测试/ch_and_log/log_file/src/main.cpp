#include "logfile.hpp"
#include <pthread.h>

void* producer_thread_proc(void* arg1)
{
    LogFile* arg = (LogFile*)arg1;
    int index = 0;
    while (true)
    {
        index++;
        if (index == 10) break;
        // std::cout << index << std::endl;
        arg->LogINFO("id=%d, gg=%d,hh=%d,ff=%s",index,12,10,"sdfa");
        arg->LogVERBOSE("id=%d, gg=%d,hh=%d,ff=%s",index,12,10,"sdfa");
        sleep(0.1);
    }
    return NULL;
}

int main()
{
    LogFile logfile(INFO,500);
    logfile.OpenLogFile("../my.log");
    pthread_t producer_thread_id[3];
    for (int i=0; i<3; i++)
        pthread_create(&producer_thread_id[i],NULL,producer_thread_proc,&logfile);
    pthread_t log_consumer_thread_id;
    pthread_create(&log_consumer_thread_id,NULL,LogFile::LogFileConsumer,&logfile);
    for (int i=0; i<3; i++)
        pthread_join(producer_thread_id[i],NULL);
    pthread_join(log_consumer_thread_id,NULL);
    return 0;
}
