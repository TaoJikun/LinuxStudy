// Copyright 2022 icf
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "threadPool.h"
#include "cstring"
#include "unistd.h"

#include <iostream>


ThreadPool::ThreadPool(int minNum,int maxNum){
    m_queue = new SafeQueue<Task>(100);
    do{
        m_minNum = minNum;
        m_maxNum = maxNum;
        m_busyCount = 0;
        m_aliveCount = m_minNum;

        m_threadIDs = new pthread_t[m_minNum];
        if(m_threadIDs == nullptr){
            cout<<"malloc thread_t[] failed..."<<endl;
            break;
        }

        memset(m_threadIDs,0,sizeof(pthread_t)*m_minNum);
        if(pthread_mutex_init(&m_mutex,NULL)!=0 || pthread_cond_init(&m_notEmpty,NULL)!=0){
            cout<<"init mutex or condition fail..."<<endl;
            break;
        }

        for(int i=0;i<minNum;i++){
            pthread_create(&m_threadIDs[i],NULL,worker,this);
            cout << "create child thread, ID: " << to_string(m_threadIDs[i]) << endl;
        }

        pthread_create(&m_managerId, NULL, manager, this);
    }while(0);
}

ThreadPool::~ThreadPool(){
    m_shutDown = true;

    pthread_join(m_managerId,NULL);

    //唤醒所有活的线程
    for(int i=0;i<m_aliveCount;i++){
        pthread_cond_signal(&m_notEmpty);
    }

    if(m_queue){
        delete m_queue;
    }
    if(m_threadIDs){
        delete m_threadIDs;
    }

    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_notEmpty);
}

//addTask
void ThreadPool::addTask(Task tk){
    if(m_shutDown)
        return ;
    m_queue->push(tk);
    pthread_cond_signal(&m_notEmpty);
}


//get the number of alive threads 
int ThreadPool::getAliveThreadCount(){
    int threadNum = 0;
    pthread_mutex_lock(&m_mutex);
    threadNum = m_aliveCount;
    pthread_mutex_unlock(&m_mutex);
    return threadNum;
}

//get the number of Busy thread
int ThreadPool::getBusyThreadCount(){
    int busyNum = 0;
    pthread_mutex_lock(&m_mutex);
    busyNum = m_busyCount;
    pthread_mutex_unlock(&m_mutex);
    return busyNum;
}

void* ThreadPool::worker(void* arg){
        ThreadPool* pool = static_cast<ThreadPool*>(arg);
    // working
    while (true){
        // mutex of access task queue
        pthread_mutex_lock(&pool->m_mutex);
        // if the task queue is empty, the worker thread block
        while (pool->m_queue->size() == 0 && !pool->m_shutDown){
            //cout << "thread " << to_string(pthread_self()) << " waiting..." << endl;
            // block thread
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_mutex);

            // after unblock, decide whether to destroy thread
            if (pool->m_exitCount > 0){
                pool->m_exitCount--;
                if (pool->m_aliveCount > pool->m_minNum){
                    pool->m_aliveCount--;
                    pthread_mutex_unlock(&pool->m_mutex);
                    pool->threadExit();
                }
            }
        }
        // whether the thread poll is shutdown
        if (pool->m_shutDown){
            pthread_mutex_unlock(&pool->m_mutex);
            pool->threadExit();
        }

        // take one task from task queue
        Task task;
        pool->m_queue->pop(task);
        // worker thread+1
        pool->m_busyCount++;
        // unlock thread pool
        pthread_mutex_unlock(&pool->m_mutex);
        // run task
        //cout << "thread " << to_string(pthread_self()) << " start working..." << endl;
        task.function(task.arg);
        task.arg = nullptr;

        // task complete
        //cout << "thread " << to_string(pthread_self()) << " end working...";
        pthread_mutex_lock(&pool->m_mutex);
        pool->m_busyCount--;
        pthread_mutex_unlock(&pool->m_mutex);
    }

    return nullptr;
}

void* ThreadPool::manager(void * arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while(!pool->m_shutDown){
        sleep(5);
        pthread_mutex_lock(&pool->m_mutex);
        int queueSize = pool->m_queue->size();
        int liveNum = pool->m_aliveCount;
        int busyNum = pool->m_busyCount;
        pthread_mutex_unlock(&pool->m_mutex);

        const int NUMBER = 2;

        if(queueSize>liveNum && liveNum<pool->m_maxNum){
            pthread_mutex_lock(&pool->m_mutex);
            int num = 0;
            for(int i=0;i<pool->m_maxNum && num<NUMBER && pool->m_aliveCount<pool->m_maxNum;i++){
                if(pool->m_threadIDs[i]==0){
                    pthread_create(&pool->m_threadIDs[i],NULL,worker,pool);
                    num++;
                    pool->m_aliveCount++;
                }
            }
            pthread_mutex_unlock(&pool->m_mutex);
        }

        if(busyNum*2 < liveNum && liveNum > pool->m_minNum){
            pthread_mutex_lock(&pool->m_mutex);
            pool->m_exitCount = NUMBER;
            pthread_mutex_unlock(&pool->m_mutex);
            for (int i = 0; i < NUMBER; ++i){
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }

    }
    return nullptr;
}

void ThreadPool::threadExit(){
    pthread_t tid = pthread_self();
    for(int i=0;i<m_maxNum;i++){
        if(m_threadIDs[i]==tid){
            cout<<"threadExit() function: thread"<<to_string(pthread_self())<<"exiting..."<<endl;
            m_threadIDs[i] = 0;
            break;;
        }
    }
    pthread_exit(NULL);
}
