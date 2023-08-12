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

#include "../common/safeQueue.h"


#include "pthread.h"
#include <iostream>
#include <unistd.h>
#include <thread>
using namespace std;

SafeQueue<int> *m_queue = new SafeQueue<int>(50);


void* put(void* arg){
    while(true){
        m_queue->push(5);
        cout<<"put: "<<m_queue->size()<<endl;
    }
    return nullptr;
}

void *pop(void* arg){
    while(true){
        int t;
        m_queue->pop(t);
        cout<<"pop: "<<m_queue->size()<<endl;
    }
    return nullptr;
}

 
void* t1(void* arg) // 入队
{
    while (1)
    {
        for (int i = 1; i <= 1000; i++)
        {
            sleep(1);
            //cout << "qLength: " << m_queue->size()  << endl;

            m_queue->push(i);
            cout <<"tid  "<<gettid()<< " size " << m_queue->size() << " put:  " << i << endl;
            
        }
        //cout << "put " << m_queue->size() << " top:  " << i << endl;
    }
}
 
 
 
void* t2(void* arg) // 100ms 发送一次队首数据
{
    while (1)
    {
        while (m_queue->size() != 0)
        {
            sleep(1);
            int a;
            //cout << "qLength: " << m_queue->size() << " top:  " << a << endl;
            m_queue->pop(a);
            cout << "tid  "<<gettid()<<" size " << m_queue->size() << " pop:  " << a << endl;
        }
    }
}



int main(){
    pthread_t* product_threads = new pthread_t(10);
    
    for(int i=0;i<10;i++){
        product_threads[i] = pthread_create(&product_threads[i],NULL,&t1,NULL);
        //pthread_join(product_threads[i],NULL);
    }
    

    pthread_t* consumer_threads = new pthread_t(10);
    
    for(int i=0;i<10;i++){
        consumer_threads[i] = pthread_create(&consumer_threads[i],NULL,&t2,NULL);
        //pthread_join(consumer_threads[i],NULL);
    }

    for(int i=0;i<10;i++){
        pthread_join(product_threads[i],NULL);
        pthread_join(consumer_threads[i],NULL);
    }

    // delete product_threads;
    // delete consumer_threads;
    // while(1){

    // }

    pthread_exit(NULL);
    delete product_threads;
    delete consumer_threads;
    // thread th1(t1);
    // thread th2(t2);
    // th1.join(); //等待th1执行完
    // th2.join(); //等待th2执行完

    return 0;
}
