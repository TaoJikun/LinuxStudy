//
// Created by admin on 2022/5/13.
//

#include "manager.h"

Manager * Manager::m_SingleInstance = NULL;
std::mutex Manager::m_mutex;

string Manager::getWhichCacheServer(string key){
    return "";
}

void Manager::init(){
    server->start();
}

Manager* Manager::GetInstance(){
    if(m_SingleInstance==NULL){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_SingleInstance==NULL){
            m_SingleInstance = new Manager();
            m_SingleInstance->hash = new NodeConHash();
            m_SingleInstance->server = new NetServer(7000, 3, 2000, false, 4);        
        }
    }
    return m_SingleInstance;
}

void Manager::deleteInstance(){
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_SingleInstance){
        delete m_SingleInstance;
        m_SingleInstance = NULL;
    }
}

//===================================================================client===========================================================//
void Manager::clientGetDistribution(struct sockaddr_in &_addr){
    json j;
    j["machineType"] = MASTER;
    j["req_type"] = DISTRIBUTION_RESPONSE;
    vector<string> allIpPorts;
    for (size_t i = 0; i < ipPortLs.size(); i++){
        string oneCacheServer = ipLs[i]+":"+to_string(portLs[i]);
        allIpPorts.push_back(oneCacheServer);
    }
    j["data"]["iplist"] = allIpPorts;
    //std::cout<<"clientGetDistribution"<<std::endl;
    std::string s = j.dump();
    for(auto &c:server->users_){
        if(c.second.getAddr().sin_port==_addr.sin_port && c.second.getAddr().sin_addr.s_addr == _addr.sin_addr.s_addr){
            c.second.connnectionSend(s);
        }
    }
}


//===========================================cacheServer==========================================//
void Manager::cacheServerKeepAlive(struct sockaddr_in &_addr){
    //std::cout<<"Manager::cacheServerkeepAlive"<<std::endl;
    pair<string,int> pp =getIpAndPort(_addr);
    string strIp = pp.first;
    int  client_port = pp.second;
    string strPort = std::to_string(client_port);
    string ipPortStr = strIp+":"+strPort;
    
    unique_lock<mutex> lock(m_sourceMutex);
    int fd = findFdAcrAddr(_addr);
    
    //已经建立连接的心跳包!!!
    if(cacheServersMp.find(ipPortStr)!=cacheServersMp.end()){
        //重置定时器
        server->timer_->update(fd,2500);
    }else{
        ipPortLs.push_back(ipPortStr);
        ipLs.push_back(strIp);
        portLs.push_back(client_port);
        cacheServersMp.insert({ipPortStr,0});
        //添加定时器
        server->timer_->addTimer(fd,2500,std::bind(&Manager::someCacheServerLost,this,_addr));
        //通知CacheServer
        notifyCacheServer(ADD_CACHE_SERVER);
    }
}

//===========================================cacheServer==========================================//
void Manager::shutDownOneMachine(struct sockaddr_in& _addr){
    deleteOneMachine(_addr);
    //有CacheServer掉线!!!
    notifyCacheServer(SHUTDOWN_CACHE);
}


//===========================================cacheServer==========================================//
void Manager::someCacheServerLost(struct sockaddr_in&_addr){
    deleteOneMachine(_addr);
    //有CacheServer掉线!!!
    notifyCacheServer(REFLESH_IP);
}

//===========================================cacheServer==========================================//
void Manager::notifyCacheServer(int type){
    //refersh:
    hash->RefreshIpList(ipPortLs);

    json j;
    j["machineType"] = MASTER;
    j["req_type"] = type;
    vector<string> allIpPorts;
    for (size_t i = 0; i < ipPortLs.size(); i++){
        string oneCacheServer = ipLs[i]+":"+to_string(portLs[i]);
        allIpPorts.push_back(oneCacheServer);
    }
    j["data"]["iplist"] = allIpPorts;

    std::string s = j.dump();
    std::cout<<"notifyCacheServer "<<type<<"cacaeServer "<<ipPortLs.size()<<"erver->users_ "<<server->users_.size()<<std::endl;

    //互斥锁，批处理所有的
    unique_lock<mutex> lock(m_sourceMutex);
    for(auto &c:server->users_){
        string ip = c.second.getIp();
        int port = c.second.getPort();
        string ipport = ip+":"+to_string(port);
        //这个users_里面可能会有非常多的连接，不一定全部都是cacheServer的连接
        if(cacheServersMp.find(ipport)!=cacheServersMp.end()){
            c.second.connnectionSend(s);
        }
    }
}

//===========================================Tool==========================================//
void Manager::deleteOneMachine(const struct sockaddr_in&_addr){
    //delete
    std::cout<<"deleteOneMachine"<<std::endl;
    unique_lock<mutex> lock(m_sourceMutex);

    pair<string,int> pp =getIpAndPort(_addr);
    string strIp = pp.first;
    int  client_port = pp.second;
    string strPort = std::to_string(client_port);
    string ipPortStr = strIp+":"+strPort;

    for(auto it = ipPortLs.begin();it!=ipPortLs.end();it++){
        if((*it)==ipPortStr){
            ipPortLs.erase(it);
            break;
        }
    }
    for(auto it = ipLs.begin();it!=ipLs.end();it++){
        if((*it)==strPort){
            ipLs.erase(it);
            break;
        }
    }
    for(auto it = portLs.begin();it!=portLs.end();it++){
        if((*it)==client_port){
            portLs.erase(it);
            break;
        }
    }
    for(auto it = cacheServersMp.begin();it!=cacheServersMp.end();it++){
        if((*it).first==ipPortStr){
            cacheServersMp.erase(it);
            break;
        }
    }
}

//===========================================Tool==========================================//
int Manager::findFdAcrAddr(struct sockaddr_in & _addr){
    for(auto &c:server->users_){
        if(c.second.getAddr().sin_port==_addr.sin_port && c.second.getAddr().sin_addr.s_addr == _addr.sin_addr.s_addr){
            return c.second.getFd();
        }
    }
    return -1;
}

//===========================================Tool==========================================//
pair<string,int> Manager::getIpAndPort(const struct sockaddr_in & _addr){
    char * client_ip = inet_ntoa(_addr.sin_addr);
    string strIp = client_ip;
    int client_port = ntohs(_addr.sin_port);
    string strPort = std::to_string(client_port);
    return {strIp,client_port};
}
