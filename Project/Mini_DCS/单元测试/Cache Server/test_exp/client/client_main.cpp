#include "client.hpp"
#include "logfile.hpp"
#include <pthread.h>
#define LOG_FILE_NAME "clientlog"

int main(int argc, char *argv[]){
    LogFile logfile(VERBOSE);
    logfile.OpenLogFile(LOG_FILE_NAME);
    logfile.LogInfo("open file %s for log",LOG_FILE_NAME);
    pthread_t log_consumer_thread_id;
    pthread_create(&log_consumer_thread_id,NULL,LogFile::LogFileConsumer,&logfile);
    logfile.LogTRACE("create pthread for writing log");
    std::vector<std::string> cacheiplist
    {
        "127.0.0.1:8000",
        "127.0.0.1:8001",
        "127.0.0.1:8002"
    };
    //std::cout << cacheiplist[0] << std::endl;
    Client cli(&logfile);

    //cli1的功能
    //cli.KeyValueWrite();

    //cli2的功能
    while(1){
        //持续拉取txt文件，直至自行退出程序
        std::string rkeyvalue = ReadTxt(KEY_FILE_NAME);
        int cut = rkeyvalue.find_last_of(' ');
        std::cout << cut << std::endl;
        std::string rkey = rkeyvalue.substr(0,cut);
        std::string rvalue = rkeyvalue.substr(cut+1);
        std::cout << rkey << std::endl;
        std::cout << rvalue << std::endl;
        std::string value = cli.KeyValueRequest(rkey);
        std::cout << "根据key:" << rkey << "求取的value为:" << value << std::endl;
        if(value == rvalue){
            std::cout << "求取正确!" << std::endl;
        }
        else std::cout << "求取错误!" << std::endl;
    }
    
    pthread_join(log_consumer_thread_id,NULL);
    return 0;
}