#include <iostream>
#include "cstring"
#include "cctype"
#include "fcntl.h"
#include "wrap.hpp"
#include "json.hpp"
#include "thrd_pool.hpp"
#include "logfile.hpp"
#include <pthread.h>
//#include <pthreadtypes.h>
#include "signal.h"
#include <algorithm>
//#include "stl_algo.h"
#include <vector>
#include <future>
using json = nlohmann::json;
//定义通信端口
#define MASTER_IP "127.0.0.1"
#define MASTER_PORT 7000
#define SPAREMASTER_IP "127.0.0.1"
#define SPAREMASTER_PORT 7001
//常数定义
#define HEART_BEAT 2
#define DISTRIBUTION_REQUEST 3
#define DISTRIBUTION_RESPOND 4
#define SHUTDOWN_CACHE 6
//#define SHUTDOWN_CACHE_MANY 8
#define ADD_CACHE 7
#define REFLESH_IP 10//探测到新的cache server心跳包，master向所有cache sever(包括新加入的cache)
#define SPARE_MASTER 8
#define REFLESH_MASTER 9
int flag1=0;//如果有新的IP加入，就把标志位置1,master向所有cache sever(包括新加入的cache)发送
int flag2=0;//主动缩容，就把标志位置1,Master向shutdown cache发送
int flag3=0;//主动缩容，就把标志位置1,Master向其他cache 发送
int flag4=0;//主动缩容，节点已经关闭，但此时还会来一个关闭节点port的心跳包，此时会误认为重连，因此这一个心跳包就利用flag4来判断,缩容后flag4=1，心跳包再来就不会写入port数组，然后flag4=0，重连就没问题了
int flag5=0;//需要关闭多个cache,
/*
flag5
0 正常
1 用户主动关闭的cache，这是需要关闭的cache都在shutportnow数组里；设置flag2=1; flag5=1;
2 检测心跳包的false状态，cache主动请求关闭自己；设置flag2=1; flag5=2;
*/
int flag6=0;//检测一直没有收到某个PORT的心跳包,主动缩容的发完之后，再发被动缩容（flag5==2），防止主动所容的时候又发送了被动缩容
pthread_mutex_t task_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;
//存储加入的client IP\port\IP:port列表
vector<string> C_IPportlist;//IP:port
vector<string> C_IPlist;//IP
vector<int> C_Portlist;//port
//存储加入的cache IP\port\IP:port列表
vector<string> IPportlist;//IP:port
vector<string> IPlist;//IP
vector<int> Portlist;//port
vector<string> shutipnow,shutportnow;//存放缩容时需要关闭的ip和port数组
int num4=0;//主动缩容shutportnow的长度
int num5=0;//每个断开的port都可能出现即使没有重连依然出现一个心跳包，所以每检测到一个就计数，直到等于num4
string shutip,shutport;//主动缩容的cache IP port
string psshutip,psshutport;//被动缩容的cache IP port
int flag9=0;
int num_none=0;
//存储心跳包里面的ip和port
string heartip,heartport;
//port-0/1，发送后为1，否则为0，以判断是否为所有的cache都发送了新的port
unordered_map<string, int> cachemap;//扩容的时候向其他cache发消息
unordered_map<string, int> cachemapshut;//缩容的时候向其他cache发消息,否则置1出现问题
//port-0/1，发送后为1，否则为0，以判断是否为所有的client都发送了新的port
//unordered_map<string, int> clientmap;
//被动缩容
unordered_map<string, int> cachestatemap;//新开线程，检测Portlist里面每个PORT对于的心跳包计数是否一直在加
unordered_map<string, int> precachestatemap;//新开线程，检测Portlist里面每个PORT对于的心跳包计数是否一直在加
//如果在多个IP里面测试，那么就需要加上这个
//unordered_map<string, int> cachestateipmap;//新开线程，检测Portlist里面每个ip对于的心跳包计数是否一直在加
//unordered_map<string, int> precachestateipmap;//新开线程，检测Portlist里面每个ip对于的心跳包计数是否一直在加

/////////////////////////////////////////
//master容灾
string masterip,masterport;
unordered_map<string, int> masterupdatemap;//向所有的cache发送备用master的IP:port

unordered_map<string, int> masterstatemap;//新开线程，检测Portlist里面每个PORT对于的心跳包计数是否一直在加
unordered_map<string, int> premasterstatemap;//新开线程，检测Portlist里面每个PORT对于的心跳包计数是否一直在加
int flag7=0;//启动新的master
int num6=0;//发送计数
int flag8=0;
/////////////////////////////////////////

//扩容发送计数
int num1=0;
//缩容向其他cache发送计数
int num2=0;
//向待关闭的cache发送，计数
int num3=0;
/*
struct pack{
    int sockfd;
    int efd;
    char* buf;
};*/

class master_handler
{
public:
    //IP分布信息
    void Distributionresponse(int clie_fd);
    //不同节点心跳包的处理
    //void Heartbeathandler(int clie_fd, char * buf);
    //关闭节点
    //void Shotdown(int clie_fd, char * buf);
    //主动扩容，新加入cache
    //void Cachejoin(int clie_fd, char * buf);
};

//IP分布信息
void master_handler::Distributionresponse(int clie_fd)
{
    pthread_rwlock_rdlock(&rw_lock);
    json data,listip;
    vector<string>  dataip;
    //////////////////test/////////
    /*IPportlist.push_back("127.0.0.2:9001");
    IPportlist.push_back("127.0.0.2:9002");
    IPportlist.push_back("127.0.0.2:9003");*/
    ///////////////////////////////
    int size=IPportlist.size();
    listip["type"]=DISTRIBUTION_RESPOND;
    for(int i=0;i<size;i++)
    {
        dataip.push_back(IPportlist[i]);
    }
    data["iplist"]=dataip;
    listip["data"]=data;
    std::string str_out = listip.dump();
    str_out += "\0";
    //clie_fd,通信的文件描述符，accept () 函数的返回值
    //(char *)str_out.data(): 传入参数，要发送的字符串
    Write(clie_fd, (char *)str_out.data(), str_out.length()+1);
    //Close(clie_fd);
    pthread_rwlock_unlock(&rw_lock);
}
//不同节点心跳包的处理
/*void master_handler::Heartbeathandler(int clie_fd, char * buf)
{
    
    pthread_rwlock_unlock(&rw_lock);
    //
    //处理心跳包
    //如果某个节点长时间没发，则缩容
    pthread_rwlock_unlock(&rw_lock);
}*/
void delayms(const int ms)
{
	struct timeval delay;
	
	delay.tv_sec = ms / 1000;
	delay.tv_usec = (ms % 1000 ) * 1000;

	select(0, NULL, NULL, NULL, &delay);
}
void refreship(int clie_fd)
{
    pthread_rwlock_rdlock(&rw_lock);
    json sum,data;
    if(flag4==1){
        sum["type"]=REFLESH_IP;
    }
    else if(flag1==1){
        sum["type"]=ADD_CACHE;
    }
    else if(flag2==1){
        sum["type"]=SHUTDOWN_CACHE;
    }
    else if(flag3==1){
        sum["type"]=REFLESH_IP;
    }
    data["iplist"]=IPportlist;
    sum["data"]=data;
    string buf1 = sum.dump();
    buf1 += "\0";
    delayms(500);
    Write(clie_fd, (char *)buf1.data(), buf1.length());
    pthread_rwlock_unlock(&rw_lock);
}

void *heartstate(void* arg1)//这个函数用来检测cache是否正常
{
    LogFile* arg = (LogFile*)arg1;
    while(1)
    {
        if(cachestatemap.size()>0)
        {
            precachestatemap=cachestatemap;
            delayms(1500);
            for(auto iter = cachestatemap.begin(); iter != cachestatemap.end(); iter++)
            {
                auto iterr = precachestatemap.find(iter->first);
                if(iterr!=precachestatemap.end())
                {
                    if(iter->second <= iterr->second)   //发现某个cache不正常
                    {
                        psshutport=iter->first;
                        psshutip=heartip;
                        if(flag6==0)
                        {
                            if(psshutport!=shutport)//防止重复关闭,主动关闭了，又被动关闭
                            {
                                flag6=1;
                            }
                        }
                    }
                    else if((iter->second) > 1000)
                    {
                        iter->second=0;
                    }
                }
            }
        }
    }
}

json updatemaster()//master心跳包
{
    json data, masterstate_json;
    //string str="127.0.0.1:7000";
    data["state"] = true;
    data["IP"]="127.0.0.1:7000";
    data["iplist"]=IPportlist;
    masterstate_json["type"] = SPARE_MASTER;
    masterstate_json["data"] = data;
    return masterstate_json;
}
void *masterupdate(void *arg1)//向备份master发送心跳包
{
    json heartbeat_json;
    struct sockaddr_in serv_addr;
    signal(SIGPIPE, SIG_IGN);
    bzero(&serv_addr,sizeof(serv_addr));
    socklen_t serv_addr_len;
    //int cfd = Socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SPAREMASTER_IP, &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(SPAREMASTER_PORT);
    int cfd = Socket_connect(true, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //Connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //auto flags = fcntl(cfd, F_GETFL); //将已连接的套接字设置为非阻塞模式
    //fcntl(cfd,F_SETFL,flags|O_NONBLOCK);
    while(1)
    {
        int n;
        heartbeat_json = updatemaster();
        string buf = heartbeat_json.dump();
        buf+="\0";
        n = Write(cfd, (char *)buf.data(), buf.length());
        if(n<0)
            cfd = Socket_connect(false, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        delayms(100);
    }
    //close(cfd);}
}
void* testallshut(void *arg1)
{
    LogFile* arg = (LogFile*)arg1;
    while(1)
    {
        int shutflag=0;
        unordered_map<string, int> precacheshutmap;
        if(cachestatemap.size()>0)
        {
            precacheshutmap=cachestatemap;
            delayms(1500);
            for(auto iter = cachestatemap.begin(); iter != cachestatemap.end(); iter++)
            {
                auto iterr = precacheshutmap.find(iter->first);
                if(iterr!=precacheshutmap.end())
                {
                    if(iter->second != iterr->second)   //发现某个cache不正常
                    {
                        shutflag=1;
                    }
                }
            }
            if(shutflag == 0)
            {
                IPportlist.clear();
                arg->LogInfo("delete all cache");
            }
        }
        
    }
}