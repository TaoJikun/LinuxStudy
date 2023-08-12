//
// Created by admin on 2022/5/18.
//

#include "nodeConHash.h"

//添加一个物理机
void NodeConHash::addPhysicalMachine(string ip_port){
    //add
    allPhysicalMachine.push_back(ip_port);
    //复制倍数
    for(int i=0;i<virtualCopys;i++){
        string temp_ipPort = ip_port+to_string(i);
        uint32_t ret_Hash = conHash.hash(temp_ipPort);
        //做一个双向映射
        mp_itoIpPort[ret_Hash] = ip_port;
    }
}

//删除一个物理机
void NodeConHash::deletePhysicalMachine(string ip_port){
    //delete
    for(auto it = allPhysicalMachine.begin();it!=allPhysicalMachine.end();it++){
        if(*it == ip_port){
            allPhysicalMachine.erase(it);
        }
    }
    //删除倍数
    for(int i=0;i<virtualCopys;i++){
        string temp_ipPort = ip_port+to_string(i);
        uint32_t  ret_hash = conHash.hash(temp_ipPort);
        mp_itoIpPort.erase(ret_hash);
    }
}

//获取key对应的物理机,顺时针查找对应的机器
string NodeConHash::findPhysicalAccordKey(string key){
    // //find
    uint32_t ret_Hash = conHash.hash(key);
    auto it = mp_itoIpPort.lower_bound(ret_Hash);
    
    if(it==mp_itoIpPort.end()){
        return (*mp_itoIpPort.begin()).second;
    }

    return (*it).second;
}

//刷新对应的ipList
void NodeConHash::RefreshIpList(vector<string> allipls){
    allPhysicalMachine.clear();
    mp_itoIpPort.clear();

    int n = allipls.size();
    for(int i=0;i<n;i++){
        addPhysicalMachine(allipls[i]);
    }
}

//根据key获取对应的缓存服务器
string NodeConHash::GetServerIndex(string key){
    return findPhysicalAccordKey(key);
}
