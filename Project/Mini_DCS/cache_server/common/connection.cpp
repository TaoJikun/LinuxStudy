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

#include "connection.h"

std::atomic<int> Connection::userCount;
bool Connection::isET;

Connection::Connection(){
    fd_ = -1;
    addr_ = {0};
    isClose_ = true;
}

Connection::~Connection(){
    closeConnect();
}

void Connection::initConnection(int sockFd,const sockaddr_in & addr){
    assert(sockFd>0);
    userCount++;
    addr_ = addr;
    fd_ = sockFd;
    readBuffer_.initPtr();
    writeBuffer_.initPtr();
    isClose_ = false;
}

//处理关闭连接的事件:但这儿就出现了bug、尽量不要在服务端关闭文件描述符!!!
void Connection::closeConnect(){
    //TODO: 需要关闭映射的文件
    if(isClose_ == false){
        //std::cout<<"isclose:::::::"<<std::endl;
        isClose_ = true;
        userCount--;
        close(fd_);
    }
}

int Connection::getFd()const{
    return fd_;
}

struct sockaddr_in Connection::getAddr() const{
    return addr_;
}

const char* Connection::getIp()const{
    return inet_ntoa(addr_.sin_addr);
}

int Connection::getPort() const{
    return addr_.sin_port;
}

//et 模式下循环读取缓冲区内容...
//每个ET模式下的文件描述符都应该是阻塞的
ssize_t Connection::readBuffer(int *saveErrno){
    ssize_t len = -1;
    do{
        len = readBuffer_.readFd(fd_,saveErrno);
        if(len<=0){ 
            break;
        }
    }while(isET);
    return len;
}

ssize_t Connection::writeBuffer(int *saveErrno){
    ssize_t len = -1;
    do{
         len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuffer_.initPtr();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuffer_.updateReadPtr(len);
        }
    }while(isET||writeBytes()>10240);
    return len;
}

/*
1) 初始化responce
2) 根据buffer组装报文并传入writeBuffer
*/

bool Connection::handleConnec(){
    std::cout<<"Connecion::handleConnec "<<std::endl;

    request_.init();
    needSend = false;

    if(readBuffer_.readableBytes() <= 0) {
        //std::cout<<"readBuffer is empty!"<<std::endl;
        return false;
    }else if(request_.parse(readBuffer_,addr_)) {

    }else{
        //记录未能解析的buffer_
        std::cout<<"Connection::handleConnec 400!!!"<<std::endl;
        return false;
    }

   return needSend;
}

void Connection::connnectionSend(std::string msg){
    setNeedSend();
    std::cout<<"Connection::connnectionSend"<<std::endl;
    response_.makeResponse(writeBuffer_,msg);
    iov_[0].iov_base = const_cast<char*>(writeBuffer_.curReadPtr());
    iov_[0].iov_len = writeBuffer_.readableBytes();
    iovCnt_ = 1;
}

void Connection::setNeedSend(){
    needSend = true;
}
