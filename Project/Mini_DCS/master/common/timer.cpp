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

#include "timer.h"

#include <iostream>

void TimerManager::addTimer(int id,int timeout,const TimeoutCallBack& cb){
    assert(id>=0);
    size_t i;
    if(ref_.count(id)==0){
        //
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id,Clock::now()+MS(timeout),cb});
        shiftup(i);
    }else{
        //
        i = ref_[id];
        heap_[i].expire = Clock::now()+MS(timeout);
        heap_[i].cb = cb;
        if(!shiftdown(i, heap_.size())) {
            shiftup(i);
        }
    }
}

void TimerManager::handle_expired_event(){
    /* 清除超时结点 */
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimeNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expire - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();
        pop();
    }
}

int TimerManager::getNextHandle(){
    //std::cout<<"getNextHandle"<<std::endl;
    handle_expired_event();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expire - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}

void TimerManager::update(int id,int timeout){
     /* 调整指定id的结点 */
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expire = Clock::now() + MS(timeout);;
    shiftdown(ref_[id], heap_.size());
}

void TimerManager::work(int id){
    /* 删除指定id结点，并触发回调函数 */
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimeNode node = heap_[i];
    node.cb();
    del_(i);
}

void TimerManager::pop(){
    assert(!heap_.empty());
    del_(0);
}

void TimerManager::clear(){
    ref_.clear();
    heap_.clear();
}

void TimerManager::del_(size_t index){
    /* 删除指定位置的结点 */
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        swapNode(i, n);
        if(!shiftdown(i, n)) {
            shiftup(i);
        }
    }
    /* 队尾元素删除 */
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void TimerManager::shiftup(size_t i){
    assert(i>=0 && i<heap_.size());
    size_t j = (i-1)/2;
    while(j>=0){
        if(heap_[j]<heap_[i]){
            break;
        }
        swapNode(i,j);
        i = j;
        j = (i-1)/2;
    }
}

bool TimerManager::shiftdown(size_t index, size_t n){
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        swapNode(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void TimerManager::swapNode (size_t i, size_t j){
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}
