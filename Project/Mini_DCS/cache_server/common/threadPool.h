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



#ifndef CACHESYSTEM_THREADPOOL_H
#define CACHESYSTEM_THREADPOOL_H

#include "pthread.h"

#include "safeQueue.h"
#include "task.h"

// class Task;
// template<typename T> class SafeQueue;

class ThreadPool{
public:
    ThreadPool(int min,int max);
    ~ThreadPool();
    //addTask
    void addTask(Task tk);
    //get the number of Busy thread
    int getBusyThreadCount();
    //get the number of alive threads 
    int getAliveThreadCount();

private:
    static void* worker(void* arg);
    static void* manager(void * arg);
    void threadExit();
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_notEmpty;
    //核心线程数
    int m_minNum;
    //最大线程数
    int m_maxNum;
    //忙碌的线程数
    int m_busyCount;
    //存活的线程数
    int m_aliveCount;
    //
    int m_exitCount;
    bool m_shutDown = false;

    pthread_t* m_threadIDs;
    pthread_t m_managerId;

    SafeQueue<Task>* m_queue;
};

#endif