#include "cache_server.hpp"
#include "HeartBeat.hpp"
void *heart_beat(void *arg){
    Heartbeat heartbeat_hd((struct heartbeat_struct *)arg);
    heartbeat_hd.heartbeat_connect(false);
    while(1){
        heartbeat_hd.heartbeat_send();
        Sleep_ms(100);
    }
    return nullptr;
}

int main(){
    //create heartbeat thread
    auto ipport_list = std::make_shared<std::vector<std::string>>();
    pthread_t tid_heartbeat;
    struct heartbeat_struct heartbeat_arg;
    heartbeat_arg.ipport_list = ipport_list;
    Pthread_create(&tid_heartbeat, nullptr, heart_beat, &heartbeat_arg);  
    
    //receive as server
    for(;;){
        sleep(100);
    }
    return 0;
}

