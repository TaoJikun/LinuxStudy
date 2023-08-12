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

#include "request.h"
#include "../src/manager.h"


Request::Request(){
    init();
}

void Request::init(){

}

//需要判断能否解析成功
bool Request::parse(Buffer& buff,const struct sockaddr_in &_addr){
    //std::cout<<"Request::parse: "<<std::endl;
    //buff.printContent();

    std::string s = buff.AlltoStr();
    json j = json::parse(s);
    req_data_ = j.get<req>();
    addr_ = _addr;

    switch(req_data_.machineType){
        case CACHE_SERVER:
            request_cacheServer_();
            break;
        case CLIENT:
            request_client_();
            break; 
        case MASTER:
            request_master_();
            break;
        default:
            return false;
    }

    return true;
}

//处理缓存服务器请求
bool Request::request_cacheServer_(){
    //std::cout<<"Request::request_cacheServer_: "<<std::endl;
    switch(req_data_.req_type){
        case KEEP_ALIVE:
            cacheServer_KeepAlive();
            break;
        default:
            return false;
    }
    return true;
}

//缓存服务器心跳包
void Request::cacheServer_KeepAlive(){
    //获取当前的ip port即可!!!
    //managet::get
    Manager::GetInstance()->cacheServerKeepAlive(addr_);
}



//处理客户端请求
bool Request::request_client_(){
    std::cout<<"Request::request_client_: "<<std::endl;
    switch(req_data_.req_type){
         case DISTRIBUTION_REQUEST:
            client_GetDistritution();
            break;
        default:
            return false;
    }
}

void Request::client_GetDistritution(){
    Manager::GetInstance()->clientGetDistribution(addr_);
}


//从masster
bool Request::request_master_(){
    std::cout<<"Request::request_client_: "<<std::endl;
}
