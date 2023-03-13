#include <iostream>
#include <string>
#include <sys/epoll.h>

#define MAX_CONNECT 4096 // 最大连接数


// 客户端的信息
class Client{
public:
    int sockfd;
    std::string name;
};



int main()
{
    
}