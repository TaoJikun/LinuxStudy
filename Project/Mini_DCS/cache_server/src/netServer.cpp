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

#include "netServer.h"


NetServer::NetServer(int port,int trigMode,int timesOutMs,bool optLinger,int threadNum):
    port_(port),openLinger_(optLinger),timeOutMs_(timesOutMs),isClose_(false),
    timer_(new TimerManager()),threadPool_(new ThreadPool(threadNum,threadNum)),epoller_(new Epoller())
{
    Connection::userCount = 0;

    initEventMode(trigMode);
    if(!initSocket_()){
        isClose_ = true;
    }
}

NetServer::~NetServer(){
    close(listenFd_);
    isClose_ = true;

    //free(srcDir_);
}

//main.cpp 中使用这个函数
void NetServer::start(){
    //
    int timeMS=1;//epoll wait timeout==-1就是无事件一直阻塞
    if(!isClose_) 
    {
        std::cout<<"============================Server Start!==========================="<<std::endl;
    }

    while(!isClose_)
    {
        if(timeOutMs_>0)
        {
            timeMS=timer_->getNextHandle();
        }

        //阻塞的最小毫秒
        int eventCnt=epoller_->wait(timeMS);
        //std::cout<<"----------------eventCnt:----------------------------------------- "<<eventCnt<<std::endl;

        for(int i=0;i<eventCnt;++i)
        {
            //获取发生的事件描述符
            int fd=epoller_->getEventFd(i);
            //获取发生的事件
            uint32_t events=epoller_->getEvents(i);
            //监听事件
            if(fd==listenFd_){
                handleListen();
                std::cout<<fd<<"============================is listening!============================"<<std::endl;
            }
            //关闭连接事件
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                std::cout<<fd<<"============================handle closeConnect!============================"<<std::endl;
                closeConnection(&users_[fd]);
            }
            //读事件
            else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                //std::cout<<fd<<"============================handle read============================-"<<std::endl;
                handleRead(&users_[fd]);
            }
            //写事件
            else if(events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                std::cout<<fd<<"============================handle write============================"<<std::endl;
                handleWrite(&users_[fd]);
            } 
            //异常事件
            else {
                std::cout<<"============================Unexpected event============================"<<std::endl;
            }
        }
    }
}

bool NetServer::initSocket_(){
    int ret;
    struct sockaddr_in addr;
    if(port_>65536 || port_<1024){
        std::cout<<"port number error!"<<std::endl;
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger optLinger  = {0};
    if(openLinger_){
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET,SOCK_STREAM,0);
    if(listenFd_<0){
        std::cout<<"create socket error!"<<std::endl;
        return false;
    }

    ret = setsockopt(listenFd_,SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger));
    if(ret<0){
        close(listenFd_);
        std::cout<<"init linger erroe!"<<std::endl;
    }

    int optVal = 1;
    ret = setsockopt(listenFd_,SOL_SOCKET,SO_REUSEADDR,(const void*)&optVal,sizeof(int));
    if(ret == -1) {
        //std::cout<<"set socket setsockopt error !"<<std::endl;
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        //std::cout<<"Bind Port"<<port_<<" error!"<<std::endl;
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        //printf("Listen port:%d error!\n", port_);
        close(listenFd_);
        return false;
    }
    ret = epoller_->addFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        //printf("Add listen error!\n");
        close(listenFd_);
        return false;
    }
    setFdNonblock(listenFd_);
    //printf("Server port:%d\n", port_);
    return true;
}

//trigMode = 3
void NetServer::initEventMode(int trigMode){
    listenEvent_ = EPOLLRDHUP;
    connectionEvent_ = EPOLLONESHOT | EPOLLRDHUP;

    switch(trigMode){
        case 0:
            break;
        case 1:
            connectionEvent_ |= EPOLLET;
            break;
        case 2:
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            listenEvent_ |= EPOLLET;
            connectionEvent_ |=EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            connectionEvent_ |= EPOLLET;
            break;
    }

    Connection::isET = (connectionEvent_ & EPOLLET);
}

//NetServer::handleListen调用
void NetServer::addConnection(int fd,sockaddr_in addr){
    assert(fd>0);
    users_[fd].initConnection(fd,addr);
    //长时间没有更新则删除
    if(timeOutMs_>0){
        timer_->addTimer(fd,timeOutMs_,std::bind(&NetServer::closeConnection,this,&users_[fd]));
    }
    epoller_->addFd(fd,EPOLLIN|connectionEvent_);
    setFdNonblock(fd);
}

//关闭连接
void NetServer::closeConnection(Connection* client){
    assert(client);
    std::cout<<"NetServer::closeConnection..."<<std::endl;
    epoller_->delFd(client->getFd());
    client->closeConnect();
}

//从半连接队列中新建connection
void NetServer::handleListen(){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do{
        int fd = accept(listenFd_,(struct sockaddr*)&addr,&len);
        if(fd<=0){
            //Err
            return ;
        }else if(Connection::userCount>=MAX_FD){
            sendErr(fd,"server busy");
            std::cout<<"client is full!"<<std::endl;
            return ;
        }
        addConnection(fd,addr);        
    }while(listenEvent_ & EPOLLET);
}

//将写任务添加至线程池
void NetServer::handleWrite(Connection* client){
    assert(client);
    extentTime(client);
    Task tk;
    tk.arg = client;
    tk.function = std::bind(&NetServer::onWrite_,this,std::placeholders::_1);
    std::cout<<"handleWrite..."<<std::endl;
    threadPool_->addTask(tk);
}

//将读任务添加至线程池
void NetServer::handleRead(Connection* client){
    assert(client);
    extentTime(client);
    Task tk;
    tk.arg = client;
    tk.function = std::bind(&NetServer::onRead_,this,std::placeholders::_1);
    std::cout<<"handleRead..."<<std::endl;
    threadPool_->addTask(tk);
}

//内部调用函数，表示正在读socket，填入读缓冲区
void NetServer::onRead_(void* arg){
    Connection* client = (Connection*)arg; 
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->readBuffer(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        std::cout<<"do not read data!"<<std::endl;
        closeConnection(client);
        return;
    }
    onProcess_(client);
    //client->handleConnec();
}

//内部调用函数，表示正在写
void NetServer::onWrite_(void* arg){
    Connection* client = (Connection*)arg; 
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->writeBuffer(&writeErrno);
    if(client->writeBytes() == 0) {

         /* 传输完成 */
      //  if(client->isKeepAlive()) {
            onProcess_(client);
            //resetEpollIn(client);
            return;
        //}
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLOUT);
            return;
        }
    }
    //closeConnection(client);
}

void NetServer::onProcess_(Connection* client){
    //client->handleConnec();
    if(client->handleConnec()) {
        //socket缓冲区可写
        epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLOUT);
    } 
    else {
        //socket缓冲区可读
        epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLIN);
    }
}

void NetServer::resetEpollOut(Connection* client){
    epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLOUT);
}

void NetServer::resetEpollIn(Connection* client){
    epoller_->modFd(client->getFd(), connectionEvent_ | EPOLLIN);
}


void NetServer::sendErr(int fd,const char* info){
    assert(fd>0);
    int ret = send(fd,info,strlen(info),0);
    if(ret<0){
        std::cout<<"send error to client"<<fd<<" error!"<<std::endl;
    }
    close(fd);
}

void NetServer::extentTime(Connection* client){
    assert(client);
    if(timeOutMs_>0){
        timer_->update(client->getFd(),timeOutMs_);
    }
}

//设置为非阻塞状态
int NetServer::setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
