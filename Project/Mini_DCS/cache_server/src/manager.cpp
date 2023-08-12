//
// Created by admin on 2022/5/13.
//

#include "manager.h"
#include "config.h"

Manager * Manager::m_SingleInstance = NULL;
std::mutex Manager::m_mutex;

string Manager::getWhichCacheServer(string key){
    return "";
}

void Manager::init(){
    //开启线程池服务
    
    server->start();
}

Manager* Manager::GetInstance(){
    if(m_SingleInstance==NULL){
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_SingleInstance==NULL){
            m_SingleInstance = new Manager();
            m_SingleInstance->server = new NetServer(CACAHESERVER2_PORT, 3, 2000000, false, 2);        
            m_SingleInstance->heartBeat = new HeartBeat();
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

void Manager::cacheServerKeepAlive(struct sockaddr_in &_addr){
    char * client_ip = inet_ntoa(_addr.sin_addr);
    string strIp = client_ip;
    int client_port = ntohs(_addr.sin_port);
    string strPort = std::to_string(client_port);
    string ipPortStr = strIp+strPort;

    unique_lock<mutex> lock(m_sourceMutex);

    if(cacheServersMp.find(ipPortStr)!=cacheServersMp.end()){

    }else{
        ipPortLs.push_back(ipPortStr);
        ipLs.push_back(strIp);
        portLs.push_back(client_port);
        cacheServersMp.insert({ipPortStr,0});

        notifyCacheServer();
    }
}

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

void Manager::notifyCacheServer(){
    json j;
    for(int i=0;i<ipLs.size();i++){
        j.push_back({ipLs[i], portLs[i]});
    }
    std::string s = j.dump();

    for(auto &c:server->users_){
        c.second.connnectionSend(s);
        //server->resetEpollOut(&c.second);
    }
}