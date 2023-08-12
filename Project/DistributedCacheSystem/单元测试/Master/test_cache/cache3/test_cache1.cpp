#include <iostream>
#include "cstring"
#include "cctype"
#include "fcntl.h"
#include "wrap.hpp"
#include "json.hpp"
#include <pthread.h>
//#include <pthreadtypes.h>
#include "signal.h"
#include <algorithm>
//#include "stl_algo.h"
#include <vector>
#include <future>
using json = nlohmann::json;
using namespace std;
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

#define MASTER_IP "127.0.0.1"
#define MASTER_PORT 7000
void delayms(const int ms)
{
	struct timeval delay;
	
	delay.tv_sec = ms / 1000;
	delay.tv_usec = (ms % 1000 ) * 1000;

	select(0, NULL, NULL, NULL, &delay);
}
json updatemaster()//master心跳包
{
    json req,ipli;
    req["type"]=HEART_BEAT;
    ipli["iplist"]="127.0.0.1:8002";
    ipli["state"]=true;
    req["data"]=ipli;
    return req;
}
int main(void)
{
	//发送心跳包
	char buf[BUFSIZ];
    json heartbeat_json;
    struct sockaddr_in serv_addr;
    signal(SIGPIPE, SIG_IGN);
    bzero(&serv_addr,sizeof(serv_addr));
    socklen_t serv_addr_len;
    //int cfd = Socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, MASTER_IP, &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(MASTER_PORT);
    int cfd = Socket_connect(true, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    int flag1=0;
    while(flag1==0)
    {
        int n;
        heartbeat_json = updatemaster();
        string str_out = heartbeat_json.dump();
        str_out+="\0";
        n = Write(cfd, (char *)str_out.data(), str_out.length()+1);
        if(n<0)
            cfd = Socket_connect(false, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        delayms(1000);

        memset(buf, 0, sizeof(buf));
        ssize_t len = unblock_read_net(cfd, buf, sizeof(buf));
		if(0<len)
        {
            printf("%s\n",buf);
            //json info = json::parse(buf);
            json info;
            if(json::accept(buf))
            info = json::parse(buf);
            if(info["type"]==SHUTDOWN_CACHE)
               flag1=1; 
        }
    }
    Close(cfd);
	return 0;
}
