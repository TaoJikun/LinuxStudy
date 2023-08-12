#ifndef HEARTBEAT_HPP
#define HEARTBEAT_HPP
#include "cache_server.hpp"
class Heartbeat{
private:
    std::shared_ptr<std::vector<std::string>> ipport_list;
    std::shared_ptr<std::unordered_map<std::string ,std::vector<std::string>>> sendout_bk;
    pthread_mutex_t ipchange_lock;
    struct sockaddr_in serv_addr;
    json heartbeat_json, iplist_json;
    char buf_rdlist[BUFSIZ];
    std::string buf;
    int cfd;
public:
    pthread_t takeout_allval[200];
    Heartbeat(struct heartbeat_struct *heartbeat_arg);
    ~Heartbeat();
    void heartbeat_connect(bool flag);
    void heartbeat_send();
    int is_anycommond();
    void shutdown_handler();
    void add_handler();
    void reflesh_handler();
};

json pack_json_heartbeat(){
    json data, heartbeat_json;
    std::string number = curen_addr;
    data["iplist"] = number;
    data["state"] = true;
    heartbeat_json["type"] = HEART_BEAT;
    heartbeat_json["data"] = data;
    return heartbeat_json;
}
Heartbeat::Heartbeat(struct heartbeat_struct *heartbeat_arg){
    ipchange_lock = PTHREAD_MUTEX_INITIALIZER;
    sendout_bk = std::make_shared<std::unordered_map<std::string ,std::vector<std::string>>>();
    ipport_list = heartbeat_arg->ipport_list;
    heartbeat_json = pack_json_heartbeat();
    buf = heartbeat_json.dump();
}
Heartbeat::~Heartbeat(){
    pthread_mutex_unlock(&ipchange_lock);
    pthread_mutex_destroy(&ipchange_lock);
    pthread_mutex_destroy(&reflesh_master_lock);
    Close(cfd);
}
void Heartbeat::heartbeat_connect(bool flag){
    int n=-1;
    do{
        if(flag){
            Close(cfd);
            Sleep_ms(100);
        }
        signal(SIGPIPE, SIG_IGN);
        bzero(&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        pthread_mutex_lock(&reflesh_master_lock);
        auto m_addr = get_ipport(master_addr);
        std::string& MASTER_IP = m_addr.ip;
        int& MASTER_PORT = m_addr.port;
        pthread_mutex_unlock(&reflesh_master_lock);
        inet_pton(AF_INET, (char *)MASTER_IP.data(), &serv_addr.sin_addr.s_addr);
        serv_addr.sin_port = htons(MASTER_PORT);
        auto info_con = Socket_connect((struct sockaddr *)&serv_addr, sizeof(serv_addr));
        cfd = info_con.cfd;
        n = info_con.n;
        flag = true;
    }while(n<0);
    std::string logtemp_info = "connect with master "+master_addr;
    auto flags = fcntl(cfd,F_GETFL,0);
    fcntl(cfd,F_SETFL,flags|O_NONBLOCK);
}
void Heartbeat::heartbeat_send(){
    int n;
    buf += "\0";
    n = Write(cfd, (char *)buf.data(), buf.length()+1);
    if(n<0){
        heartbeat_connect(true);
    }
}
int Heartbeat::is_anycommond(){
    return 0;
}
void Heartbeat::shutdown_handler(){
}
void Heartbeat::add_handler(){
}
void Heartbeat::reflesh_handler(){
}
#endif /* HEARTBEAT_HPP */
