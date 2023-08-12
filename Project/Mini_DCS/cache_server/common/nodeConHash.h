//
// Created by admin on 2022/5/18.
//

#ifndef DEMO_NODECONHASH_H
#define DEMO_NODECONHASH_H

#include "consistentHash.h"
#include <vector>
#include <string>
#include <map>
using namespace std;

class NodeConHash{
public:
    //刷新ipLists
    void RefreshIpList(vector<string> allipls);
    //获取key对应的ip
    string GetServerIndex(string key);
public:
    //添加一个物理机
    void addPhysicalMachine(string ip_port);
    //删除一个物理机
    void deletePhysicalMachine(string ip_port);
    //获取key对应的物理机,顺时针查找对应的机器
    string findPhysicalAccordKey(string key);
    //物理节点的复制倍数
    int virtualCopys = 1048576;
    //所有的物理机
    vector<string> allPhysicalMachine;
    //int--->string
    map<uint32_t,string> mp_itoIpPort;
    //哈希函数
    ConsistentHash conHash;
};


#endif //DEMO_NODECONHASH_H
