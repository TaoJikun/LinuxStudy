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

#ifndef NETSERVER_H
#define NETSERVER_H


#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <functional>
#include <arpa/inet.h>

#include "../common/connection.h"
#include "../common/epoll.h"
#include "../common/threadPool.h"
#include "../common/timer.h"


class NetServer{
public:
    NetServer(int port,int trigMode,int timeOutMs,bool optLinger,int threadNum);
    ~NetServer();
    void start();

    void onProcess_(Connection* client);

    void resetEpollOut(Connection* client);

    void resetEpollIn(Connection* client);


private:
    //初始化socket_
    bool initSocket_();
    //
    void initEventMode(int trigMode);

    void addConnection(int fd,sockaddr_in addr);
    void closeConnection(Connection* client);

    void handleListen();
    void handleWrite(Connection* client);
    void handleRead(Connection* client);

       void onRead_(void* arg);
       void onWrite_(void* arg);

    //void onProcess_(Connection* client);

    void sendErr(int fd,const char* info);
    void extentTime(Connection* client);

    static const int MAX_FD = 65536;
    static int setFdNonblock(int fd);


    int port_;
    int timeOutMs_;
    bool isClose_;
    int listenFd_;
    bool openLinger_;

    //char* srcDir_;

    uint32_t listenEvent_;
    uint32_t connectionEvent_;

    //std::unique_ptr<TimerManager> timer_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<Epoller> epoller_;
public:
    std::unique_ptr<TimerManager> timer_;
    std::unordered_map<int,Connection> users_;
};

#endif