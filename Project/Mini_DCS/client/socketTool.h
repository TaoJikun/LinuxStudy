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

#ifndef socketTool_H
#define socketTool_H

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

#include "sys/time.h"
#include "sys/epoll.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "fcntl.h"

#include "arpa/inet.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#define RAND(min,max) (rand()%((int)(max+1)-(int)(min))+(int)(min))

void Sleep_ms(int n);

void perr_exit(const char* s);

//建立socket套接字
int Socket(int family,int type,int protocol);

//建立连接
int Connect(int fd,const struct sockaddr*sa,socklen_t len);

//接收连接
int Accept(int fd,struct sockaddr* sa,socklen_t* len);

//绑定socket套接字
int Bind(int fd,const struct sockaddr* sa,socklen_t len);

//监听socket套接字
int  Listen(int fd,int backlog);

//关闭文件套接字
int Close(int fd);

//创建epoll文件
int Epoll_create(int size);

//向epoll中添加/删除/修改需要监听的文件描述符
int Epoll_ctl(int epfd,int op,int fd,struct epoll_event* event);

//监听epoll中的文件描述符
int Epoll_wait(int epfd,struct epoll_event* events,int maxevents,int timeout);

//创建新线程、调用回调函数
void Pthread_create(pthread_t* tid,const pthread_attr_t* attr,void*(*func)(void*),void* arg);
 
//向服务端发起连接
int Socket_connect(bool first_call,const struct sockaddr* sa,socklen_t len);

struct info_conn{
    int cfd;
    int n;
};

info_conn Socket_connect(const struct sockaddr* sa,socklen_t len);

struct ipport_pair{
    int     port;
    std::string  ip;
};

ipport_pair get_ipport(std::string addr);

std::string RandStr(const int len);

std::string ReadText(std::string filename);

//读
ssize_t Read(int fd,char* ptr,size_t nbyte);

//读一次
ssize_t ReadOnce(int cfd,char* buf,int size);
//重复读3次
ssize_t Unblock_read_net(int cfd,char* buf,int size);

//向文件描述符中写数据
ssize_t Write(int fd,const void* ptr,size_t nbyte);

#endif
