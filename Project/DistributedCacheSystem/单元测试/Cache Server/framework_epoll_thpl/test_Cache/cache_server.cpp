#include "cache_server.hpp"
#include "WriteRead.hpp"
int main(){
    auto key_addr = std::make_shared<ConsistentHash>(100);
    key_addr->RefreshIPList({curen_addr});
    auto ipport_list = std::make_shared<std::vector<std::string>>();
    auto LC = std::make_shared<LRUCache>(100);
    auto LC_BK = std::make_shared<LRUCache>(100);
    auto ThrPl = std::make_shared<ThreadPool>(10,20);
    //receive as server
    WriteRead WR_CACHE(ThrPl, LC, LC_BK, key_addr);
    WR_CACHE.WR_listen();
    for(;;){
        int nready = WR_CACHE.WR_listenWait(500);
        if(nready<=0)
            continue;
        for(int i=0;i<nready;i++){
            if(WR_CACHE.is_listenfd(i)){
                WR_CACHE.WR_listenHandler();
            }
            else
                WR_CACHE.WR_cfdHandler(i);
        }
    }
    return 0;
}

