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

#ifndef CACHESYSTEM_SAFEQUEUE_H
#define CACHESYSTEM_SAFEQUEUE_H

//https://www.freeaihub.com/post/93796.html

#include <vector>
#include <string>
#include <map>
using namespace  std;

#include <pthread.h>
#include <vector>
#include <iostream>

typedef unsigned long ulong;

template<typename T>
class SafeQueue{
public:
    explicit SafeQueue(unsigned int capacity)
        :m_capacity(capacity+1)
        ,m_queue(m_capacity)
        ,m_head(0)
        ,m_tail(0)
    {
        pthread_mutex_init(&m_mutex,NULL);
        pthread_cond_init(&m_notFull,NULL);
        pthread_cond_init(&m_notEmpty,NULL);
    }

    ~SafeQueue(){
        m_queue.clear();
        pthread_cond_destroy(&m_notFull);
        pthread_cond_destroy(&m_notEmpty);
        pthread_mutex_destroy(&m_mutex);
    }

    unsigned long size(){
        return (m_tail+m_capacity-m_head)%m_capacity;
    }

    void push(const T&e){
        pthread_mutex_lock(&m_mutex);
        while(isFull()){
            pthread_cond_wait(&m_notFull,&m_mutex);
        }
        m_queue[m_tail] = e;
        m_tail = (m_tail+1)%(m_capacity);
        //释放不是空的信号
        pthread_cond_signal(&m_notEmpty);
        pthread_mutex_unlock(&m_mutex);
        //std::cout<<m_tail<<" "<<m_head<<" "<<size()<<" "<<e<<endl;
    }

    void pop(T& t){
        pthread_mutex_lock(&m_mutex);
        while(isEmpty()){
            //等待不是空的信号
            pthread_cond_wait(&m_notEmpty,&m_mutex);
        }
        T res = m_queue[m_head];
        m_head = (m_head+1)%(m_capacity);
        //释放出不是满的信号
        pthread_cond_signal(&m_notFull);
        pthread_mutex_unlock(&m_mutex);
        t = res;
    }

private:
    //xxxx
    pthread_mutex_t m_mutex;
    pthread_cond_t m_notFull;
    pthread_cond_t m_notEmpty;

    volatile unsigned long m_head;
    volatile unsigned long m_tail;

    unsigned long m_capacity;
    std::vector<T> m_queue;

    inline bool isEmpty(){
        return m_tail == m_head;
    }

    inline bool isFull(){
        return (m_tail+1)%m_capacity == m_head;
        //return (m_head + m_capacity - m_tail) % (m_capacity + 1) == 0;
    }
};

#endif
