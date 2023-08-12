#ifndef THRDPOOL_HPP
#define THRDPOOL_HPP
 #include "pthread.h"
 #include "queue"
 #include "iostream"
 #include "unistd.h"
 #include "cstring"
 using namespace std;
// define tast struct
using callback = void(*)(void*);
struct Task{
    Task(){
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void* arg){
        function = f;
        this->arg = arg;
    }
    callback function;
    void* arg;
};

// task queue
class TaskQueue{
public:
    TaskQueue();
    ~TaskQueue();

    // add task
    void addTask(Task& task);
    void addTask(callback func, void* arg);

    // take out one task
    Task takeTask();

    // get the number of task in the current queue
    inline int taskNumber(){
        return m_queue.size();
    }

private:
    pthread_mutex_t m_mutex;    // mutex
    std::queue<Task> m_queue;   // task queue
};

TaskQueue::TaskQueue(){
    pthread_mutex_init(&m_mutex, NULL);
}

TaskQueue::~TaskQueue(){
    pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::addTask(Task& task){
    pthread_mutex_lock(&m_mutex);
    m_queue.push(task);
    pthread_mutex_unlock(&m_mutex);
}

void TaskQueue::addTask(callback func, void* arg){
    pthread_mutex_lock(&m_mutex);
    Task task;
    task.function = func;
    task.arg = arg;
    m_queue.push(task);
    pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::takeTask(){
    Task t;
    pthread_mutex_lock(&m_mutex);
    if (m_queue.size() > 0){
        t = m_queue.front();
        m_queue.pop();
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}

class ThreadPool{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

    // add task
    void addTask(Task task);
    // get the number of busy thread
    int getBusyNumber();
    // get the number of alive thread
    int getAliveNumber();

private:
    // task function of woker thread
    static void* worker(void* arg);
    // task funnction of manager thread
    static void* manager(void* arg);
    void threadExit();

private:
    pthread_mutex_t m_lock;
    pthread_cond_t m_notEmpty;
    pthread_t* m_threadIDs;
    pthread_t m_managerID;
    TaskQueue* m_taskQ;
    int m_minNum;
    int m_maxNum;
    int m_busyNum;
    int m_aliveNum;
    int m_exitNum;
    bool m_shutdown = false;
};

ThreadPool::ThreadPool(int minNum, int maxNum){
    m_taskQ = new TaskQueue;
    do {
        // init threadpool
        m_minNum = minNum;
        m_maxNum = maxNum;
        m_busyNum = 0;
        m_aliveNum = minNum;

        // allocate memory to the thread array according to the maximum limit of threads
        m_threadIDs = new pthread_t[maxNum];
        if (m_threadIDs == nullptr){
            cout << "malloc thread_t[] fail...." << endl;;
            break;
        }
        // init
        memset(m_threadIDs, 0, sizeof(pthread_t) * maxNum);
        // init mutex condition
        if (pthread_mutex_init(&m_lock, NULL) != 0 ||\
            pthread_cond_init(&m_notEmpty, NULL) != 0){
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        /////////////////// create thread //////////////////
        // create a thread according to the minimum number of threads
        for (int i = 0; i < minNum; ++i){
            pthread_create(&m_threadIDs[i], NULL, worker, this);
            cout << "create child thread, ID: " << to_string(m_threadIDs[i]) << endl;
        }
        // create manager thread
        pthread_create(&m_managerID, NULL, manager, this);
    } while (0);
}

ThreadPool::~ThreadPool(){
    m_shutdown = 1;
    // destroy manager thread
    pthread_join(m_managerID, NULL);
    // wake all custom thread
    for (int i = 0; i < m_aliveNum; ++i){
        pthread_cond_signal(&m_notEmpty);
    }

    if (m_taskQ) delete m_taskQ;
    if (m_threadIDs) delete[]m_threadIDs;
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_notEmpty);
}

void ThreadPool::addTask(Task task){
    if (m_shutdown)
        return;
    // add task
    m_taskQ->addTask(task);
    // wake worker thread
    pthread_cond_signal(&m_notEmpty);
}

int ThreadPool::getAliveNumber(){
    int threadNum = 0;
    pthread_mutex_lock(&m_lock);
    threadNum = m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

int ThreadPool::getBusyNumber(){
    int busyNum = 0;
    pthread_mutex_lock(&m_lock);
    busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}


// task function of worker thread
void* ThreadPool::worker(void* arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    // working
    while (true){
        // mutex of access task queue
        pthread_mutex_lock(&pool->m_lock);
        // if the task queue is empty, the worker thread block
        while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown){
            cout << "thread " << to_string(pthread_self()) << " waiting..." << endl;
            // block thread
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);

            // after unblock, decide whether to destroy thread
            if (pool->m_exitNum > 0){
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum){
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }
        // whether the thread poll is shutdown
        if (pool->m_shutdown){
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        // take one task from task queue
        Task task = pool->m_taskQ->takeTask();
        // worker thread+1
        pool->m_busyNum++;
        // unlock thread pool
        pthread_mutex_unlock(&pool->m_lock);
        // run task
        cout << "thread " << to_string(pthread_self()) << " start working..." << endl;
        task.function(task.arg);
        task.arg = nullptr;

        // task complete
        cout << "thread " << to_string(pthread_self()) << " end working...";
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }

    return nullptr;
}


// manager task function
void* ThreadPool::manager(void* arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (!pool->m_shutdown){
        sleep(5);
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_aliveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        // create thread
        const int NUMBER = 2;
        if (queueSize > liveNum && liveNum < pool->m_maxNum){
            // lock thread pool
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER\
                && pool->m_aliveNum < pool->m_maxNum; ++i){
                if (pool->m_threadIDs[i] == 0){
                    pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                    num++;
                    pool->m_aliveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        // Destroy redundant threads
        // busy thread*2 < alive thread && alive thread > minimum thread
        if (busyNum * 2 < liveNum && liveNum > pool->m_minNum){
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            for (int i = 0; i < NUMBER; ++i){
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }
    }
    return nullptr;
}

// thread exit
void ThreadPool::threadExit(){
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; ++i){
        if (m_threadIDs[i] == tid){
            cout << "threadExit() function: thread " 
                << to_string(pthread_self()) << " exiting..." << endl;
            m_threadIDs[i] = 0;
            break;
        }
    }
    pthread_exit(NULL);
}
#endif /* THRDPOOL_HPP */





