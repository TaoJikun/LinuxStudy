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

#ifndef CACHESYSTEM_TASK_H
#define CACHESYSTEM_TASK_H


//using callback = void(*)(void*);



#include <functional>
#include <iostream>
#include <memory>

typedef std::function<void(void*)> callback;


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


#endif