#ifndef HEART_BEAT
#define HEART_BEAT

#include <iostream>
#include "../common/socketTool.h"
#include "../common/json.h"
using json = nlohmann::json;

class HeartBeat{
public:
    
enum MACHINE_TYPE{
    CACHE_SERVER,  //0
    CLIENT,        //1
    MASTER,        //2
};

enum REQ_TYPE{
    KEEP_ALIVE,
};

enum CLIENT_REQTYPE{
    DISTRIBUTION_REQUEST,
    DISTRIBUTION_RESPONSE,
};

public:
    HeartBeat();
    ~HeartBeat(){};
    friend void* start(void*arg);
    int WriteData(int& cfd,json js);
    int connectTask(ipport_pair oneip,int &cfd);
    void heartBeatSend();
    void HeartStop();
    void initSocket_();
    //master的连接描述符
    int masterfd_=-1;
    //一直重连即可!!!
    std::string master_ipport="127.0.0.1:7000";
    int port;
    //
    struct sockaddr_in serv_addr;
    struct timeval timeout={3,0};

    pthread_t m_thread;
};

#endif