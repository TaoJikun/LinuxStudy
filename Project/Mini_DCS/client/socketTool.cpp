
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

#include "socketTool.h"

void Sleep_ms(int n){
    struct timeval delay;
    delay.tv_sec  = 0;
    delay.tv_usec = n*1000;
    select(0,NULL,NULL,NULL,&delay);
}

void perr_exit(const char* s){
    perror(s);
    exit(-1);
}

//建立socket套接字
int Socket(int family,int type,int protocol){
    int fd = socket(family,type,protocol);
    if(fd<0){
        perr_exit("socket error");
    }
    return fd;
}

//建立连接
int Connect(int fd,const struct sockaddr*sa,socklen_t len){
    int n = connect(fd,sa,len);
    if(n<0){
        perr_exit("connect error");
    }
    return n;
}


//接收连接
int Accept(int fd,struct sockaddr* sa,socklen_t* len){
    int n;
    do
    {
        n = accept(fd,sa,len);
        if(n>0){
            return n;
        }
    }while (errno==ECONNABORTED || errno == EINTR);
    perr_exit("accept error");
    return -1;
}

//绑定socket套接字
int Bind(int fd,const struct sockaddr* sa,socklen_t len){
    int n = bind(fd,sa,len);
    if(n<0){
        perr_exit("bind error");
    }
    return n;
}

//监听socket套接字
int  Listen(int fd,int backlog){
    int n = listen(fd,backlog);
    if(n<0){
        perr_exit("close error");
    }
    return n;
}

//关闭文件套接字
int Close(int fd){
    int n = close(fd);
    if(n<0){
        perr_exit("close error");
    }
    return n;
}

//创建epoll文件
int Epoll_create(int size)
{
    int efd=epoll_create(size);
    if(efd<0)
        perr_exit("epoll_create error");
    return efd;
}

//向epoll中添加/删除/修改需要监听的文件描述符
int Epoll_ctl(int epfd,int op,int fd,struct epoll_event* event)
{
    int res=epoll_ctl(epfd,op,fd,event);
    if(res<0)
        perr_exit("epoll_ctl error");
    return res;
}

//监听epoll中的文件描述符
int Epoll_wait(int epfd,struct epoll_event* events,int maxevents,int timeout)
{
    int n=epoll_wait(epfd,events,maxevents,timeout);
    if(n<0)
        perr_exit("epoll_wait error");
    return n;
}

//创建新线程、调用回调函数
void Pthread_create(pthread_t* tid,const pthread_attr_t* attr,void*(*func)(void*),void* arg){
    int n = pthread_create(tid,attr,func,arg);
    if(n==0){
        return ;
    }
    errno = n;
    perr_exit("pthread_create error");
}

//向服务端发起连接
int Socket_connect(bool first_call,const struct sockaddr* sa,socklen_t len){
    int n,fd;
    do{
        if(!first_call){
            close(fd);
            sleep(1);
        }
        fd = Socket(AF_INET,SOCK_STREAM,0);
        n = Connect(fd,sa,len);
        first_call = false;
    }while(n<0);
    auto flag = fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,flag|O_NONBLOCK);
    return fd;
}


info_conn Socket_connect(const struct sockaddr* sa,socklen_t len){
    struct info_conn info_con;
    int fd=Socket(AF_INET,SOCK_STREAM,0);
    int n= connect(fd,sa,len);
    info_con.cfd=fd;
    info_con.n=n;
    return info_con;
}


ipport_pair get_ipport(std::string addr)
{
    struct ipport_pair res;
    int cut=addr.find_last_of(':');
    res.ip=addr.substr(0,cut);
    res.port=stoi(addr.substr(cut+1));
    return res;
}

std::string RandStr(const int len)
{
    struct timeval timeSeed;
    gettimeofday(&timeSeed,NULL);
    srand(1000000*timeSeed.tv_sec+timeSeed.tv_usec);
    std::string ans;
    int i;
    for(i=0;i<len;i++)
    {
        char c;
        switch ((rand()%3))
        {
            case 1:
                c=RAND('A','Z');
                break;
            case 2:
                c=RAND('a','z');
                break;
            default:
                c=RAND('0','9');
        }
        ans+=c;
    }
    ans[++i]='\0';
    return ans;
}

std::string ReadText(std::string filename)
{
    int line;
    std::ifstream text;
    text.open(filename,std::ios::in);
    if(!text.is_open())
        std::cout<<"Open file failure"<<std::endl;
    std::vector<std::string> vec;
    while(!text.eof())
    {
        std::string inbuf;
        getline(text,inbuf,'\n');
        vec.push_back(inbuf);
    }
    std::cout<<"keylist总共有"<<vec.size()-1<<"行，读取第几行数据？"<<std::endl;
    std::cin>>line;
    return vec[line-1];
}

//读
ssize_t Read(int fd,char* ptr,size_t nbyte)
{
    ssize_t n;
    int j=0;
    do
    {
        Sleep_ms(100);
        do
        {
            n=read(fd,ptr,nbyte);
            if(n>=0)
                return n;
        }while(errno==EINTR);
        j++;
        if(j>2)
            break;
    }while(n<=0);
    perr_exit("read error");
    return -1;
}

//读一次
ssize_t ReadOnce(int cfd,char* buf,int size){
    ssize_t n;
    do{
        n = read(cfd,buf,size);
        if(n>=0){
            return n;
        }
    }while(errno==EINTR||errno==EAGAIN);
    return -1;
}

//重复读3次
ssize_t Unblock_read_net(int cfd,char* buf,int size){
    std::string res = "";
    bool flag = false;
    ssize_t rtn = 0;
    int n = 0;
    do{
        n = read(cfd,buf,size);
        if(n>0 && !flag){
            for(int i=0;i<n;i++){
                if(buf[i]=='\0'){
                    flag = true;
                    break;
                }
                if(!flag){
                    res+=buf[i];
                    rtn++;
                }
            }
        }
    }while(n>0 || errno==EINTR);

    if(rtn==0){
        return n;
    }

    int i=0;
    for(auto c:res){
        buf[i++] = c;
    }
    buf[i] = '\0';
    
    return rtn;
}

//向文件描述符中写数据
ssize_t Write(int fd,const void* ptr,size_t nbyte)
{
    ssize_t n;
    do
    {
        n=write(fd,ptr,nbyte);
        if(n>=0)
            return n;
    }while(errno==EINTR||errno==EAGAIN);
    return -1;
}

