#ifndef CACHESYSTEM_TIMER_H
#define CACHESYSTEM_TIMER_H


#include <deque>
#include <unordered_map>
#include <ctime>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <assert.h>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

class TimeNode{
public:
    int id;
    TimeStamp expire;
    TimeoutCallBack cb;

    bool operator<(const TimeNode& t){
        return expire<t.expire;
    }
};

class TimerManager{
    typedef std::shared_ptr<TimeNode> sp_timeNode;
public:
    TimerManager(){
        heap_.reserve(64);
    }
    ~TimerManager(){
        clear();
    }
    void addTimer(int id,int timeout,const TimeoutCallBack& cb);

    void handle_expired_event();

    int getNextHandle();

    void update(int id,int timeout);

    void work(int id);

    void pop();

    void clear();

private:
    void del_(size_t i);
    void shiftup(size_t i);
    bool shiftdown(size_t index, size_t n);
    void swapNode (size_t i, size_t j);


    std::vector<TimeNode> heap_;
    std::unordered_map<int,size_t> ref_;
};



#endif // !CACHESYSTEM_TIMER_H
