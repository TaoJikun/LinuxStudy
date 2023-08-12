#ifndef HEARTBEAT_HPP
#define HEARTBEAT_HPP
#include "cache_server.hpp"
class iplistchange_class{
private:
    std::shared_ptr<ConsistentHash> key_addr;
    std::shared_ptr<std::vector<std::string>> ipport_list;
    std::shared_ptr<ThreadPool> ThrPl;
    std::shared_ptr<LRUCache> LC;
    int cfd;
    std::shared_ptr<std::unordered_map<std::string, std::vector<std::string>>> ipport_and_key;
    struct task_connect_oth task_arg;
    bool exit_flag;
public:
    iplistchange_class(struct iplistchange_struct *iplistchage);
    ~iplistchange_class();
    void organize_data();
    void takeout_alldata();
};
class taskconnect_class{
private:
    std::shared_ptr<LRUCache> LC;
    std::string addr;
    struct sockaddr_in cache_oth_addr;
    std::shared_ptr<std::unordered_map<std::string, std::vector<std::string>>> ipport_and_key;
    int cachfd;
public:
    taskconnect_class(task_connect_oth *task_arg);
    ~taskconnect_class();
    int net_connect();
    void sendout_data();
};
class Heartbeat{
private:
    pthread_t tid_beforeshutdown;
    pthread_t tid_askfromothercache;
    std::shared_ptr<ConsistentHash> key_addr;
    std::shared_ptr<std::vector<std::string>> ipport_list;
    std::shared_ptr<std::unordered_map<std::string ,std::vector<std::string>>> sendout_bk;
    pthread_mutex_t ipchange_lock;
    std::shared_ptr<ThreadPool> ThrPl;
    std::shared_ptr<LRUCache> LC;
    std::shared_ptr<LRUCache> LC_BK;
    struct sockaddr_in serv_addr;
    json heartbeat_json, iplist_json;
    char buf_rdlist[BUFSIZ];
    std::string buf;
    int cfd;
    struct iplistchange_struct ipchange_arg;
public:
    task_connect_oth task_arg;
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
void* doit_iplistchange(void *arg){
    pthread_rwlock_wrlock(&shutdown_lock);
    iplistchange_class ipchange((struct iplistchange_struct *)arg);
//take out all my data to other caches
    ipchange.organize_data();
    ipchange.takeout_alldata();
    pthread_rwlock_unlock(&shutdown_lock);
    return nullptr;
}
json pack_json_heartbeat(){
    json data, heartbeat_json;
    std::string number = curen_addr;
    data["iplist"] = number;
    data["state"] = true;
    heartbeat_json["type"] = HEART_BEAT;
    heartbeat_json["data"] = data;
    return heartbeat_json;
}
void *task_connectoth(void *arg){
    pthread_mutex_lock(&transfertooth);
    taskconnect_class taskconnect((struct task_connect_oth *)arg);
    pthread_mutex_unlock(&transfertooth);
    if(taskconnect.net_connect()<0)
        return nullptr;
    taskconnect.sendout_data();
    return nullptr;
}
taskconnect_class::taskconnect_class(task_connect_oth *task_arg){
    LC = task_arg->LC;
    addr = task_arg->addr;
    ipport_and_key = task_arg->ipport_and_key;
}
taskconnect_class::~taskconnect_class(){
}
int taskconnect_class::net_connect(){
    auto ipport_s = get_ipport(addr);
    std::string& ip_oth = ipport_s.ip;
    int& port_oth = ipport_s.port;
    cachfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&cache_oth_addr,sizeof(cache_oth_addr));
    cache_oth_addr.sin_family = AF_INET;
    inet_pton(AF_INET, (const char*)ip_oth.data(), &cache_oth_addr.sin_addr.s_addr);
    cache_oth_addr.sin_port = htons(port_oth);
    int ret = connect(cachfd, (struct sockaddr *)&cache_oth_addr, sizeof(cache_oth_addr));
    auto flags = fcntl(cachfd,F_GETFL,0);
    fcntl(cachfd, F_SETFL, flags & O_NONBLOCK);
    return ret;
}
void taskconnect_class::sendout_data(){
    std::string logtemp_info = "sendout data to cache "+addr;
    logfile.LogInfo((char *)logtemp_info.data());
    for(int j=0; j<(*ipport_and_key)[addr].size(); j++){
        std::string key = (*ipport_and_key)[addr][j];
        std::string value = (*LC).get(key, false);
        json write_j = Write_oth_ClientCache(key, value);
        std::string buf = write_j.dump()+"\0";
        int n;
        //do{
            n = Write(cachfd, (const char *)buf.data(), buf.length()+1);
        //}while(n<0);
        if(n==0){
            Close(cachfd);
            return;
        }
        Sleep_ms(100);
        //sleep(1);
    }
    Close(cachfd);
}
iplistchange_class::iplistchange_class(struct iplistchange_struct *iplistchage){
    key_addr = iplistchage->key_addr;
    ipport_and_key = std::make_shared<std::unordered_map<std::string, std::vector<std::string>>>();
    ipport_list = iplistchage->ipport_list;
    ThrPl = iplistchage->ThrPl;
    LC = iplistchage->LC;
    cfd = iplistchage->cfd;
    (*key_addr).RefreshIPList(*ipport_list);
    exit_flag = iplistchage->exit_flag;
    if(exit_flag)
        std::cout<<"prepare to close..."<<std::endl;  
}
iplistchange_class::~iplistchange_class(){
}
void iplistchange_class::organize_data(){
    for(auto i = (*LC).cache.begin(); i!=(*LC).cache.end();i++){
        std::string key = i->first;
        std::string addr = (*key_addr).GetServerIndex(key);
        //std::cout<<addr<<std::endl;
        (*ipport_and_key)[addr].push_back(key);
    }
}
void iplistchange_class::takeout_alldata(){
    signal(SIGPIPE, SIG_IGN);
    task_arg.ipport_and_key = ipport_and_key;
    task_arg.LC = LC;
    //std::cout<<"============="<<std::endl;
    pthread_t takeout_allval[ipport_and_key->size()];
    int count = 0;
    int flag = -1;
    for(auto i = ipport_and_key->begin(); i!=ipport_and_key->end(); i++){
        Sleep_ms(100);
        if(curen_addr==i->first){
            flag = count++;
            continue;
        }
        pthread_mutex_lock(&transfertooth);
        task_arg.addr = i->first;
        //std::cout<<i->first<<std::endl;//test!!!!!!!!!!!!!!!!!
        Pthread_create(&takeout_allval[count++], nullptr, &task_connectoth, &task_arg);
        pthread_mutex_unlock(&transfertooth);
    }
    Sleep_ms(200);
    for(int i=0; i<ipport_and_key->size(); i++){
        if(flag==i)
            continue;
        pthread_join(takeout_allval[i], nullptr);
    }
    if(exit_flag){
        std::cout<<"close done"<<std::endl;
        exit(0);
    }
}
Heartbeat::Heartbeat(struct heartbeat_struct *heartbeat_arg){
    ipchange_lock = PTHREAD_MUTEX_INITIALIZER;
    sendout_bk = std::make_shared<std::unordered_map<std::string ,std::vector<std::string>>>();
    key_addr = heartbeat_arg->key_addr;
    ipport_list = heartbeat_arg->ipport_list;
    ThrPl = heartbeat_arg->ThrPl;
    LC = heartbeat_arg->LC;
    LC_BK = heartbeat_arg->LC_BK;
    heartbeat_json = pack_json_heartbeat();
    buf = heartbeat_json.dump();
}
Heartbeat::~Heartbeat(){
    pthread_mutex_unlock(&ipchange_lock);
    pthread_mutex_destroy(&ipchange_lock);
    pthread_mutex_destroy(&transfertooth);
    pthread_mutex_destroy(&reflesh_master_lock);
    pthread_rwlock_destroy(&shutdown_lock);
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
    logfile.LogInfo((char *)logtemp_info.data());
    auto flags = fcntl(cfd,F_GETFL,0);
    fcntl(cfd,F_SETFL,flags|O_NONBLOCK);
}
void Heartbeat::heartbeat_send(){
    int n;
    buf += "\0";
    n = Write(cfd, (char *)buf.data(), buf.length()+1);
    if(n<0){
        logfile.LogWARNING("connection with master fails");
        heartbeat_connect(true);
    }
}
int Heartbeat::is_anycommond(){
    int m=0;
    do{
        m = unblock_read_net(cfd, buf_rdlist, BUFSIZ);
        if(0<m){
            if(!(json::accept(buf_rdlist)))
                continue;
            iplist_json = json::parse(buf_rdlist);
            std::cout<<buf_rdlist<<std::endl;//test!!!!!!!!!!!!!!!!!!!!!!!
            return (int)iplist_json["type"];
        }
        else if(0==m){
            logfile.LogWARNING("connection with master fails");
            heartbeat_connect(true);
        }
    }while(0==m);
    return -1;
}
void Heartbeat::shutdown_handler(){
    pthread_mutex_lock(&ipchange_lock);
    logfile.LogInfo("receive SHUTDOWN_CACHE commond from master");
    (*ipport_list).clear();
    for(int i=0;i<iplist_json["data"]["iplist"].size();i++){
        (*ipport_list).push_back(iplist_json["data"]["iplist"][i]);
    }
    ipchange_arg.ThrPl = ThrPl;
    ipchange_arg.LC = LC;
    ipchange_arg.exit_flag = true;
    ipchange_arg.key_addr = key_addr;
    ipchange_arg.ipport_list = ipport_list;
    Pthread_create(&tid_beforeshutdown, nullptr, doit_iplistchange, &ipchange_arg);
    pthread_mutex_unlock(&ipchange_lock);
}
void Heartbeat::add_handler(){
    pthread_mutex_lock(&ipchange_lock);
    logfile.LogInfo("receive ADD_CACHE commond from master");
    (*ipport_list).clear();
    for(int i=0;i<iplist_json["data"]["iplist"].size();i++){
        (*ipport_list).push_back(iplist_json["data"]["iplist"][i]);
    }
    ipchange_arg.ThrPl = ThrPl;
    ipchange_arg.LC = LC;
    ipchange_arg.exit_flag = false;
    ipchange_arg.key_addr = key_addr;
    ipchange_arg.ipport_list = ipport_list;
    Pthread_create(&tid_askfromothercache, nullptr, doit_iplistchange, &ipchange_arg);
    pthread_mutex_unlock(&ipchange_lock);
}
void Heartbeat::reflesh_handler(){
    pthread_mutex_lock(&ipchange_lock);
    logfile.LogInfo("================receive REFLESH_IP commond from master========================");
    (*ipport_list).clear();
    std::unordered_set<std::string> ipnew;
    std::vector<std::string> key_send;
    sendout_bk->clear();

    for(int i=0; i<iplist_json["data"]["iplist"].size(); i++){
        (*ipport_list).push_back(iplist_json["data"]["iplist"][i]);
        ipnew.insert((std::string)iplist_json["data"]["iplist"][i]);
    }
    for(auto i = (*LC_BK).cache.begin(); i!=(*LC_BK).cache.end();i++){
        std::string key = i->first;
        std::string addr = (*key_addr).GetServerIndex(key);
        if(ipnew.find(addr)==ipnew.end()){
            key_send.push_back(key);
        }
    }
    (*key_addr).RefreshIPList(*ipport_list);
    for(int i=0; i<key_send.size(); i++){
        (*sendout_bk)[(*key_addr).GetServerIndex(key_send[i])].push_back(key_send[i]);
    }
    if(!(sendout_bk->empty())){
        signal(SIGPIPE, SIG_IGN);
        int count=0;        
        task_arg.LC = LC_BK;
        task_arg.ipport_and_key = sendout_bk;
        int flag=-1;
        for(auto i = sendout_bk->begin(); i!=sendout_bk->end(); i++){
            if(curen_addr==i->first){
                for(int j=0; j<(*sendout_bk)[curen_addr].size(); j++){
                    std::string temp_key = (*sendout_bk)[curen_addr][j];
                    std::string temp_val = LC_BK->get(temp_key, false);
                    LC->put(temp_key, temp_val);
                }
                flag = count++;
                continue;
            }
            Sleep_ms(100);
            pthread_mutex_lock(&transfertooth);
            task_arg.addr = i->first;
            //std::cout<<i->first<<std::endl;//test!!!!!!!!!!!!!!!!!
            Pthread_create(&takeout_allval[count++], nullptr, &task_connectoth, &task_arg);
            pthread_mutex_unlock(&transfertooth);        
        }
    }
    pthread_mutex_unlock(&ipchange_lock);
}


#endif /* HEARTBEAT_HPP */
