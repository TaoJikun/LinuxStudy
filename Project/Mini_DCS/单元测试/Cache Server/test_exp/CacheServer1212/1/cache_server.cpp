#include "cache_server.hpp"
#include "WriteRead.hpp"
#include "HeartBeat.hpp"
void *write_log(void *arg){
    LogFile* logfile = (LogFile*)arg;
    logfile->LogFileConsumer(logfile);
    return nullptr;
}

void *heart_beat(void *arg){
    Heartbeat heartbeat_hd((struct heartbeat_struct *)arg);
    heartbeat_hd.heartbeat_connect(false);
    while(1){
        heartbeat_hd.heartbeat_send();
        Sleep_ms(100);
        int commond = heartbeat_hd.is_anycommond();
        if(SHUTDOWN_CACHE==commond){
            heartbeat_hd.shutdown_handler();
            break;
        }
        else if(ADD_CACHE==commond){
            heartbeat_hd.add_handler();
        }
        else if(REFLESH_IP==commond){
            heartbeat_hd.reflesh_handler();
        }
    }
    while(1){
        sleep(100);
    }
    return nullptr;
}

int main(){
    //create log thread
    logfile.OpenLogFile("CacheServer_log");
    pthread_t tid_log;
    Pthread_create(&tid_log, nullptr, write_log, &logfile);
    //create heartbeat thread
    auto key_addr = std::make_shared<ConsistentHash>(100);
    key_addr->RefreshIPList({curen_addr});
    auto ipport_list = std::make_shared<std::vector<std::string>>();
    auto LC = std::make_shared<LRUCache>(100);
    auto LC_BK = std::make_shared<LRUCache>(100);
    auto ThrPl = std::make_shared<ThreadPool>(10,20);
    pthread_t tid_heartbeat;
    struct heartbeat_struct heartbeat_arg;
    heartbeat_arg.ThrPl = ThrPl;
    heartbeat_arg.LC = LC;
    heartbeat_arg.LC_BK = LC_BK;
    heartbeat_arg.key_addr = key_addr;
    heartbeat_arg.ipport_list = ipport_list;
    Pthread_create(&tid_heartbeat, nullptr, heart_beat, &heartbeat_arg);  
    
    //receive as server
    WriteRead WR_CACHE(ThrPl, LC, LC_BK, key_addr);
    WR_CACHE.WR_listen();
    for(;;){
        int nready = WR_CACHE.WR_listenWait(500);
        if(nready<=0)
            continue;
        pthread_rwlock_rdlock(&shutdown_lock);
        for(int i=0;i<nready;i++){
            if(WR_CACHE.is_listenfd(i)){
                logfile.LogInfo("receive new connect");
                WR_CACHE.WR_listenHandler();
            }
            else
                WR_CACHE.WR_cfdHandler(i);
        }
        pthread_rwlock_unlock(&shutdown_lock);
    }
    return 0;
}

