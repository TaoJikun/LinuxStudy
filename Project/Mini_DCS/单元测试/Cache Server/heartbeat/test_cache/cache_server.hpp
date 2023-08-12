#ifndef CACHE_SERVER_HPP
#define CACHE_SERVER_HPP
#include <cstring>
#include <cctype>
#include <fcntl.h>
#include "wrap.hpp"
#include "json.hpp"
#include <signal.h>
#include <unordered_set>

#define HEART_BEAT 2
#define KEY_VALUE_REQUEST 0
#define KEY_VALUE_RESPOND 1
#define KEY_VALUE_RESPONDBK 11
#define REFLESH_MASTER 9
#define SHUTDOWN_CACHE 6
#define ADD_CACHE 7
#define REFLESH_IP 10
#define CACHESERV_IP "127.0.0.1"
#define CACHESERV_PORT 8000
std::string master_addr = "127.0.0.1:7000";
const std::string curen_addr = CACHESERV_IP+(std::string)":"+std::to_string(CACHESERV_PORT);
using json = nlohmann::json;
struct heartbeat_struct{
    std::shared_ptr<std::vector<std::string>> ipport_list;
};
pthread_mutex_t reflesh_master_lock = PTHREAD_MUTEX_INITIALIZER;
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
