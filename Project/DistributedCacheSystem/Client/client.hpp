#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include "sys/socket.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "signal.h"
#include "json.hpp"
#include "consistenthash.hpp"
#include "wrap.hpp"
#include <fstream>
#include "logfile.hpp"
#include <pthread.h>
#include <time.h>

#define KEY_VALUE_REQUEST 0
#define KEY_VALUE_RESPOND 1
#define HEART_BEAT 2
#define DISTRIBUTION_REQUEST 3
#define DISTRIBUTION_RESPOND 4
#define CACHE_SHUNTDOWN 5
#define CACHE_JOIN 6
#define MASTER_IP "127.0.0.1:7000"
#define MASTER_IP_BACKUP "127.0.0.1:7001"
#define KEY_FILE_NAME "keydata.txt"
#define TIME_OUT 5

using json = nlohmann::json;

//template <typename T> 
class Client
{
public:
    //friend T;
    Client(void* logarg);
    Client(std::vector<std::string> iplist,void* logarg);
    ~Client();
    std::vector<std::string> alliplist;
    void KeyValueWrite();
    std::string KeyValueRequest(std::string key);
    //void TestTxtWrite();
    
private:   ///  //for test!!!
    ConsistentHash hash;
    LogFile* log;
    json Writejs(int type);
    json Writejs(std::string key);
    json ReadData(int &cfd);
    int ConnectServ(OneIPList oneip,int& cfd);
    int WriteData(int &cfd,json js);
    void DistributionRequest();
    bool CountTime();
    int kvwritecfd,kvreqcfd,mastercfd;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    time_t start,end;
    //clock_t start,end;
    struct timeval timeout = {3,0};//3s
};

Client::Client(void* logarg){
    //无初始给定值时，先向master请求一次分布
    log = (LogFile*)logarg;
    log->LogINFO("create client without initialization");
    DistributionRequest();
    //ConsistentHash hash(100);
}
Client::Client(std::vector<std::string> iplist,void* logarg){
    //本地初始化iplist分布
    log = (LogFile*)logarg;
    log->LogINFO("create client with initialization");
    alliplist.swap(iplist);
    //ConsistentHash hash(100);
    hash.RefreshIPList(alliplist);
    time(&start);
}
Client::~Client(){
}

json Client::Writejs(int type)
{
    json js, data;
    if(type == KEY_VALUE_RESPOND){
        data["key"] = Randstr(20);
        data["value"] = Randstr(200);
        data["flag"] = true;
        log->LogTRACE("create new KEY_VALUE_RESPOND json data");
    }
    else if(type == DISTRIBUTION_REQUEST){
        data = json::object();
        log->LogTRACE("create new DISTRIBUTION_REQUEST json data");
    }
    js["type"] = type;
    js["data"] = data;
    return js;
}

json Client::Writejs(std::string key)
{
    json js, data;
    data["flag"] = true;
    data["key"] = key;
    data["value"] = "";
    js["type"] = KEY_VALUE_REQUEST;
    js["data"] = data;
    log->LogTRACE("create new KEY_VALUE_REQUEST json data");
    return js;
}

int Client::ConnectServ(OneIPList oneip,int &cfd){
    //默认最多重连四次
    int j = 0;
    cfd = Socket(AF_INET, SOCK_STREAM, 0);
    // auto flags = fcntl(cfd,F_GETFL,0);
    // fcntl(cfd,F_SETFL,flags|O_NONBLOCK);
    int ret = setsockopt(cfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, (oneip.ip.c_str()), &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(oneip.port);
    int n = connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    log->LogTRACE("connect once");
    if(n<0){
        do{
            close(cfd);
            cfd = Socket(AF_INET, SOCK_STREAM, 0);
            // flags = fcntl(cfd,F_GETFL,0);
            // fcntl(cfd,F_SETFL,flags|O_NONBLOCK);
            ret = setsockopt(cfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
            n = connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            log->LogTRACE("connect again");
            sleep(1);
            if(n>=0)
                break;
            j++;
        }while(j<=3);
    }
    return n;
}

//从指定套接字读取json数据
json Client::ReadData(int &cfd){
    char bufin[BUFSIZ];
    int n;
    int j = 0;
    Sleep_ms(100);
    log->LogTRACE("read data start");
    n = Read(cfd, bufin, BUFSIZ);
    if (n<=0){
        log->LogTRACE("read data fails");
        return json::object();
    }
    log->LogTRACE("read data success");
    //Write(STDOUT_FILENO, bufin, n);//test
    if(!json::accept(bufin)){
        log->LogERROR("read no json data");
        PerrExit("no json data");
    }
    log->LogVERBOSE("read json data : %s", bufin);
    json js = json::parse(bufin);
    std::cout << "read json data: " << js << std::endl;
    return js;
}

//向指定套接字写入json数据
int Client::WriteData(int &cfd,json js){
    int nwrite;
    int j = 0;
    std::string buf = js.dump();
    buf += "\0";
    nwrite = Write(cfd, (char *)buf.data(), buf.length()+1);
    //std::cout << "write data: " << js << std::endl;
    log->LogTRACE("write data once");
    return nwrite;
}

//超过五秒钟视为超时，向master请求一次
bool Client::CountTime(){
    time(&end);
    //std::cout << "end: " << end << std::endl;
    if(end < start){
        log->LogERROR("time count error");
        PerrExit("time count error");
    }
    time_t interval = end - start;
    log->LogVERBOSE("start: %ld, end: %ld, interval: %lds",start,end,interval);
    bool _isout = (interval > TIME_OUT) ? true : false;
    return _isout;
}

//向各cache server写入数据
void Client::KeyValueWrite()
{
    int n,nwrite;
    //int t = 0;
    signal(SIGPIPE, SIG_IGN);
    //打开文件，从末尾写入keylist
    std::ofstream outfile;
    outfile.open(KEY_FILE_NAME, std::ios::app);
    log->LogINFO("open file %s for keylist",KEY_FILE_NAME);
    if(!outfile.is_open ()){
        log->LogERROR("open file %s failed",KEY_FILE_NAME);
        PerrExit("open file failed");
    }
    //随机生成
    json js = Writejs(KEY_VALUE_RESPOND); 
    std::string key = js["data"]["key"].get<std::string>();
    std::string value = js["data"]["value"].get<std::string>();
    log->LogVERBOSE("json data : key->%s \t value->%s",key.c_str(),value.c_str());
    if(CountTime() == true){
        log->LogINFO("distribution time out");
        DistributionRequest();
    }
    OneIPList oneipbf = GetIPList(hash.GetServerIndex(key));
    log->LogTRACE("server ip:%s", hash.GetServerIndex(key).c_str());
    n = ConnectServ(oneipbf,kvwritecfd);
    while(n < 0){
        close(kvwritecfd);
        sleep(1);
        DistributionRequest();
        oneipbf = GetIPList(hash.GetServerIndex(key));
        log->LogTRACE("server ip:%s", hash.GetServerIndex(key));
        n = ConnectServ(oneipbf,kvwritecfd);
    }
    log->LogINFO("set connection to server: %s:%d",oneipbf.ip.c_str(),oneipbf.port);
    nwrite = WriteData(kvwritecfd,js);
    if(nwrite < 0){
        log->LogERROR("writing data fails");
        oneipbf.port = 0;
    }
    outfile << key << ' ' << value << ' ' << oneipbf.port << std::endl; // 存一下value以测试是否正确

    //while (t<10) //for test
    while(1)
    {
        js = Writejs(KEY_VALUE_RESPOND); //随机生成
        key = js["data"]["key"].get<std::string>();
        value = js["data"]["value"].get<std::string>();
        log->LogVERBOSE("send key-value json data : key->%s \t value->%s",key.c_str(),value.c_str());
        if(CountTime() == true){
            log->LogINFO("distribution time out");
            DistributionRequest();
        }
        OneIPList oneip = GetIPList(hash.GetServerIndex(key));
        if((oneip.ip != oneipbf.ip)||(oneip.port != oneipbf.port))
        {
            Close(kvwritecfd);     //关闭之前的连接
            log->LogTRACE("close connection to server: %s:%d",oneipbf.ip.c_str(),oneipbf.port);
            n = ConnectServ(oneip,kvwritecfd);
            while(n<0){
                close(kvwritecfd);
                sleep(1);
                DistributionRequest();
                oneip = GetIPList(hash.GetServerIndex(key));
                n = ConnectServ(oneip,kvwritecfd);
            }
            log->LogINFO("set connection to server: %s:%d",oneip.ip.c_str(),oneip.port);
        }
        nwrite = WriteData(kvwritecfd,js);
        if(nwrite < 0){
            log->LogERROR("writing data fails");
            oneipbf.port = 0;
            sleep(1);
            continue;
        }
        oneipbf = oneip;
        outfile << key << ' ' << value << ' ' << oneip.port << std::endl;
        //t++;
        Sleep_ms(300);  //test
    }
    Close(kvwritecfd);
    outfile.close();
}

std::string Client::KeyValueRequest(const std::string key){
    json js = Writejs(key);
    json jsin;
    OneIPList oneip;
     int n,nwrite;
    int j = 0;
    log->LogVERBOSE("send key-value request data : key->%s",key.c_str());
    while(1){
        if(CountTime() == true){
            log->LogINFO("distribution time out");
            DistributionRequest();
        }
        oneip = GetIPList(hash.GetServerIndex(key));
        //std::cout << oneip.port << std::endl;
        
        n = ConnectServ(oneip,kvreqcfd);
        while(n < 0){
            close(kvreqcfd);
            sleep(1);
            DistributionRequest();
            oneip = GetIPList(hash.GetServerIndex(key));
            n = ConnectServ(oneip,kvreqcfd);
        }
        
        log->LogINFO("set connection to server: %s:%d",oneip.ip.c_str(),oneip.port);
        nwrite = WriteData(kvreqcfd,js);
        if(nwrite < 0){
            log->LogERROR("writing data fails");
            close(kvreqcfd);
            DistributionRequest();
            continue;
        }
        jsin = ReadData(kvreqcfd);
        if(jsin.empty()) {
            log->LogERROR("connection fails");
            close(kvreqcfd);
            PerrExit("connection with cache fails");
        }
        else if(jsin["data"]["flag"] == false) {
            log->LogERROR("data not here");
            close(kvreqcfd);
            DistributionRequest();
            j++;
            if(j > 2) PerrExit("Data Lose");
            continue;
        }
        else if(jsin["type"] == KEY_VALUE_REQUEST && jsin["data"]["key"] == key) break;
        else PerrExit("key value request returns fault");
    }
    Close(kvreqcfd);
    std::string value = jsin["data"]["value"].get<std::string>();
    std::cout << "return value: " << value << std::endl;
    return value;
}

void Client::DistributionRequest()
{
    int n,nwrite;
    OneIPList masterip = GetIPList(MASTER_IP);
    json client_to_master = Writejs(DISTRIBUTION_REQUEST);
    //std::cout << client_to_master << std::endl;
    do{
        log->LogINFO("--connection to master starts--");
        n = ConnectServ(masterip,mastercfd);
        if(n != 0){
            log->LogWARNING("--connection to master fails--");
            close(mastercfd);
            log->LogINFO("close connection to master");
            masterip = GetIPList(MASTER_IP_BACKUP);
            log->LogINFO("--connection to backup master starts--");
            n = ConnectServ(masterip,mastercfd);
            if(n != 0){
                log->LogERROR("--connection to backup master fails--");
                PerrExit("connection to master failed");
            }
        }
        log->LogINFO("--connection to master done--");
        nwrite = WriteData(mastercfd,client_to_master);
        log->LogINFO("--send distribution resquest to master--");
    }while(nwrite < 0);
    json jsin = ReadData(mastercfd); 
    if(jsin["type"] != DISTRIBUTION_RESPOND){
        log->LogERROR("distribution request returns fault");
        PerrExit("distribution request returns fault");
    }
    alliplist = jsin["data"]["iplist"].get<std::vector<std::string>>();
    if(alliplist.empty()){
        log->LogERROR("invalid distribution");
        PerrExit("invalid distribution");
    }
    hash.RefreshIPList(alliplist);
    time(&start);
    //start = clock();
    log->LogINFO("--distrbution request done--");
    std::cout << "distrbution request once" << std::endl;
    Close(mastercfd);
}

#endif /* CLIENT_HPP */