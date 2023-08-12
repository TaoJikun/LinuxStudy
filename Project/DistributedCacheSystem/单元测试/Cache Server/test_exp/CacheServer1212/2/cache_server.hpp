#ifndef CACHE_SERVER_HPP
#define CACHE_SERVER_HPP
#include <cstring>
#include <cctype>
#include <fcntl.h>
#include "wrap.hpp"
#include "json.hpp"
#include "LRU.hpp"
#include <signal.h>
#include "thrd_pool.hpp"
#include "consistenthash.hpp"
#include <unordered_set>
#include "logfile.hpp"

#define HEART_BEAT 2
#define KEY_VALUE_REQUEST 0
#define KEY_VALUE_RESPOND 1
#define KEY_VALUE_RESPONDBK 11
#define REFLESH_MASTER 9
#define SHUTDOWN_CACHE 6
#define ADD_CACHE 7
#define REFLESH_IP 10
#define CACHESERV_IP "127.0.0.1"
#define CACHESERV_PORT 8002
std::string master_addr = "127.0.0.1:7000";
const std::string curen_addr = CACHESERV_IP+(std::string)":"+std::to_string(CACHESERV_PORT);
LogFile logfile;
long long request_count = 0;
long long rcv_cli = 0;
long long rcv_bk = 0;
using json = nlohmann::json;
struct heartbeat_struct{
    std::shared_ptr<LRUCache> LC;
    std::shared_ptr<LRUCache> LC_BK;
    std::shared_ptr<ThreadPool> ThrPl;
    std::shared_ptr<ConsistentHash> key_addr;
    std::shared_ptr<std::vector<std::string>> ipport_list;
};
struct pack_taskconnect{
    int sockfd;
    int efd;
    char* buf;
    std::shared_ptr<LRUCache> LC;
    std::shared_ptr<LRUCache> LC_BK;
    std::shared_ptr<ConsistentHash> key_addr;
};
struct pack_tasklisten{
    struct sockaddr_in *cliaddr;
    int listenfd;
    int num;
    int efd;
};
struct iplistchange_struct{
    int cfd;
    bool exit_flag;
    std::shared_ptr<LRUCache> LC;
    std::shared_ptr<ThreadPool> ThrPl;
    std::shared_ptr<ConsistentHash> key_addr;
    std::shared_ptr<std::vector<std::string>> ipport_list;
};
struct task_connect_oth{
    std::string addr;
    std::shared_ptr<std::unordered_map<std::string, std::vector<std::string>>> ipport_and_key;
    std::shared_ptr<LRUCache> LC;
};
//////////////////////////////
pthread_mutex_t transfertooth = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t task_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tasklisten_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t reflesh_master_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t shutdown_lock = PTHREAD_RWLOCK_INITIALIZER;

//////////////////////////
json Write_oth_ClientCache(const std::string& key, const std::string& value){
    json client_to_cache, data;
    data["flag"] = true;
    data["key"] = key;
    data["value"] = value;
    client_to_cache["type"] = KEY_VALUE_RESPOND;
    client_to_cache["data"] = data;
    return client_to_cache;
}
json Write_bk_ClientCache(const std::string& key, const std::string& value){
    json client_to_cache, data;
    data["flag"] = true;
    data["key"] = key;
    data["value"] = value;
    client_to_cache["type"] = KEY_VALUE_RESPONDBK;
    client_to_cache["data"] = data;
    return client_to_cache;
}
struct ipport_pair{
    int port;
    std::string ip;
};
struct ipport_pair get_ipport(std::string addr){
    bool ip_order = true;
    std::string port_s; 
    struct ipport_pair res;
    for(char c:addr){
        if(c==':'){
            ip_order = !ip_order;
            continue;
        }
        if(ip_order)
            res.ip += c;
        else
            port_s+=c;
    }
    res.port = stoi(port_s);
    return res;
}
#endif /* CACHE_SERVER_HPP */
