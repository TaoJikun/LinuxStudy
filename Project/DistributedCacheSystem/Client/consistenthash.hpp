#ifndef CONSISTENTHASH_HPP
#define CONSISTENTHASH_HPP

//#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <set>
#include <map>

#define RAND(min, max) ( rand() % ((int)(max+1) - (int)(min)) + (int)(min) )

class ConsistentHash
{
public:
    ConsistentHash(int num = 100);
    ~ConsistentHash();

    static uint32_t FNVHash(std::string key);
    static std::string RandStr(const int len);
    bool RefreshIPList(const std::vector<std::string>& ipList);
    bool AddServer(const std::string& nodeIp);
    bool DeleteServer(const std::string& nodeIp);
    std::string GetServerIndex(const std::string& key);
    std::string GetBackUpServer(const std::string& key);

private:
    std::map<uint32_t, std::string>virtualNodes;
    std::map<uint32_t, std::string>serverNodes;
    int virtualNodeNum;

};


ConsistentHash::ConsistentHash(int num): virtualNodeNum(num) {}

ConsistentHash::~ConsistentHash(){}

uint32_t ConsistentHash::FNVHash(std::string key) {
    const int p = 16777619;
    uint32_t hash = 2166136261;
    for (int idx = 0; idx < key.size(); ++idx) {
        hash = (hash ^ key[idx]) * p;
    }
    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;
    if (hash < 0) {
        hash = -hash;
    }
    return hash;
}

std::string ConsistentHash::RandStr(const int len)
{
    std::string ans;
    int i;
    for (i = 0; i < len; i++)
    {
        char c;
        switch ((rand() % 3))
        {
        case 1:
            c = RAND('A', 'Z');
            break;
        case 2:
            c = RAND('a', 'z');
            break;
        default:
            c = RAND('0', '9');
        }
        ans += c;
    }
    ans[++i] = '\0';
    return ans;
}

bool ConsistentHash::RefreshIPList(const std::vector<std::string>& ipList)
{
    virtualNodes.clear();
    bool f = true;
    for (auto& ip:ipList) f = f && AddServer(ip);
    return f;
}

bool ConsistentHash::AddServer(const std::string& nodeIp)
{
    for(int i = 0; i < virtualNodeNum; i++) {
        std::stringstream nodeKey;
        nodeKey << nodeIp << "#" << i;
        uint32_t partition = FNVHash(nodeKey.str());
        virtualNodes.insert({partition, nodeIp});
    }
    uint32_t partition = FNVHash(nodeIp);
    serverNodes.insert({partition, nodeIp});
    return true;
}

bool ConsistentHash::DeleteServer(const std::string& nodeIp)
{
    for(int j = 0; j < virtualNodeNum; ++j) {
        std::stringstream nodeKey;
        nodeKey << nodeIp << "#" << j;
        uint32_t partition = FNVHash(nodeKey.str());
        auto it = virtualNodes.find(partition);
        if(it != virtualNodes.end()) { virtualNodes.erase(it); }
    }
    uint32_t partition = FNVHash(nodeIp);
    auto it = serverNodes.find(partition);
    if(it != serverNodes.end()) { serverNodes.erase(it); }
    return true;
}

std::string ConsistentHash::GetServerIndex(const std::string& key)
{
    uint32_t partition = FNVHash(key);
    auto it = virtualNodes.lower_bound(partition);
    if(it == virtualNodes.end()) {
        if (virtualNodes.empty()) {
            std::cout << "no available nodes" << '\n';
        }
        return virtualNodes.begin()->second;
    }
    return it->second;
}

std::string ConsistentHash::GetBackUpServer(const std::string& key)
{
    uint32_t partition = FNVHash(key);
    auto it = serverNodes.upper_bound(partition);
    if(it == serverNodes.end()) {
        if (serverNodes.empty()) {
            std::cout << "no available nodes" << '\n';
        }
        return serverNodes.begin()->second;
    }
    return it->second;
}


#endif // CONSISTENTHASH_HPP
