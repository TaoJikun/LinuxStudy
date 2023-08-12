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

#ifndef CONNEXTION_H
#define CONNEXTION_H

#include "buffer.h"

#include <arpa/inet.h>
#include <iostream>
#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <assert.h>

#include "request.h"
#include "response.h"

class Connection{
public:
    Connection();
    ~Connection();

    void initConnection(int socketFd,const sockaddr_in&addr);

    ssize_t readBuffer(int *saveErrno);
    ssize_t writeBuffer(int* saveErrno);

    void closeConnect();
    bool handleConnec();

    const char* getIp()const;
    int getPort()const;
    int getFd()const;
    sockaddr_in getAddr() const;


    int writeBytes(){
        return iov_[1].iov_len+iov_[0].iov_len;
    }


    void connnectionSend(std::string msg);
    void setNeedSend();
    
    // bool isKeepLive(){
    //     return request_.isKeepAlive();
    // }

    static bool isET;

    //static const char* srcDir

    static std::atomic<int> userCount;

private: 
    //连接对应的描述符
    int fd_;
    //存储连接信息的套接字
    struct sockaddr_in addr_;
    //链接是否关闭
    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];
    
    //读缓冲区
    Buffer readBuffer_; 
    //写缓冲区
    Buffer writeBuffer_;


    Request request_;
    Response response_;

    bool needSend;
};


#endif