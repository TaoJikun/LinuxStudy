#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <future>
#include <queue>

#include "threadsafe_queue_ht.h"

//using namespace std;

class join_threads
{
private:
    std::vector<std::thread>& threads;
public:
    explicit join_threads(std::vector<std::thread>&_threads):threads(_threads){
        
    }
    ~join_threads(){
        for(unsigned long i = 0;i<threads.size();i++){
            //代表该线程是可执行线程
            if(threads[i].joinable()){
                threads[i].join();
            }
        }
    }
};

class function_wrapper{
private:
    struct impl_base{
        virtual void call()=0;
        virtual ~impl_base(){}
    };

    template<typename F>
    struct impl_type:impl_base{
        F f;
        impl_type(F&& f_):f(std::move(f_)){

        }
        void call(){
            f();
        }
    };

    std::unique_ptr<impl_base> impl;
public:
    template<typename F>
    function_wrapper(F&& f):impl(new impl_type<F>(std::move(f))){

    }

    void operator()(){
        impl->call();
    }

    function_wrapper() = default;

    function_wrapper(function_wrapper&& other):impl(std::move(other.impl)){

    }

    function_wrapper& operator=(function_wrapper&& other){
        impl = std::move(other.impl);
        return *this;
    }

    function_wrapper(const function_wrapper&)=delete;
    function_wrapper(function_wrapper&)=delete;
    function_wrapper& operator=(const function_wrapper&)=delete;
};

class work_stealing_queue{
private:
    typedef function_wrapper data_type;
    std::deque<data_type> the_queue;
    mutable std::mutex the_mutex;
public:
    work_stealing_queue(){

    }
    work_stealing_queue(const work_stealing_queue& other)=delete;
    work_stealing_queue& operator=(const work_stealing_queue& other)=delete;

    void push(data_type data){
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.push_front(std::move(data));
    }

    bool empty() const{
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(data_type& res){
        std::lock_guard<std::mutex> lock(the_mutex);
        if(the_queue.empty()){
            return false;
        }
        res = std::move(the_queue.front());
        the_queue.pop_front();
        return true;
    }   

    bool try_steal(data_type & res){
        std::lock_guard<std::mutex> lock(the_mutex);
        if(the_queue.empty()){
            return false;
        }
        res = std::move(the_queue.back());
        the_queue.pop_back();
        return true;
    }
};


class thread_pool{
private:
    typedef function_wrapper task_type;
    //是否关闭线程
    std::atomic_bool done;
    //公有队列
    icf::queue<function_wrapper> work_queue;
    //线程私有队列
    std::vector<std::unique_ptr<work_stealing_queue>> queues;
    //线程
    std::vector<std::thread> threads;
    
    join_threads joiner;
    //线程私有变量
    static thread_local work_stealing_queue* local_work_queue;
    static thread_local unsigned my_index;

    //
    int taskCount = 0;

    void worker_thread(unsigned index){
        my_index = index;
        local_work_queue = queues[my_index].get();
        while(!done){
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(task_type& task){
        return local_work_queue && local_work_queue->try_pop(task);
    }

    bool pop_task_from_pool_queue(task_type& task){
        return work_queue.try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type& task){
        for(unsigned i=0;i<queues.size();i++){
            unsigned const index = (my_index+i+1)%queues.size();
            if(queues[index]->try_steal(task)){
                //std::cout<<"stealing "<<std::endl;
                return true;
            }
        }
        
        return false;
    }

public:
    thread_pool():done(false),joiner(threads){
        unsigned const thread_count = 6;// std::thread::hardware_concurrency();
        std::cout<<thread_count<<std::endl;
        try{
            for(unsigned i=0;i<thread_count;i++){
                std::cout<<thread_count<<std::endl;
                queues.push_back(std::unique_ptr<work_stealing_queue>(new work_stealing_queue));
                //threads.push_back(std::thread(&thread_pool::worker_thread,this,i));
            }
            for(unsigned i=0;i<thread_count;i++){
                threads.push_back(std::thread(&thread_pool::worker_thread,this,i));
            }
        }
        catch(...){
            std::cout<<thread_count<<std::endl;
            done  = true;
            throw;
        }
        std::cout<<thread_count<<std::endl;
    }

    ~thread_pool(){
        done = true;
    }
    
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> AddTask(FunctionType f){
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());
        //随机获取一个lock_work_queue
        taskCount++;
        int index = (taskCount%queues.size())>0?((taskCount%queues.size())):(-(taskCount%queues.size()));
        if(index<0 || index>=queues.size()){
            exit(0);
        }
        //std::cout<<index<<std::endl;
        local_work_queue = queues[index].get();

        if(local_work_queue){
            //std::cout<<"g"<<std::endl;
            local_work_queue->push(std::move(task));
        }else{
            //std::cout<<"s"<<std::endl;
            work_queue.push(std::move(task));
        }
        return res;
    }

    void run_pending_task(){
        function_wrapper task;
        if(pop_task_from_local_queue(task)||pop_task_from_pool_queue(task)||pop_task_from_other_thread_queue(task)){
                task();
        }else{
                std::this_thread::yield();
        }
    }
};

