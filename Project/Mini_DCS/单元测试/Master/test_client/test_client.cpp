#include "iostream"
#include "unistd.h"
#include "cstring"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "json.hpp"
#define SERV_IP "127.0.0.1"
#define SERV_PORT 7000
#define DISTRIBUTION_REQUEST 3
using json = nlohmann::json;
using namespace std;
void perr_exit(const char* s){
    perror(s);
    exit(-1);
}
int Socket(int family, int type, int protocol){
    int n;
    if((n=socket(family, type, protocol))<0)
        perr_exit("socket error");
    return n;
}
int Connect(int fd, const struct sockaddr* sa, socklen_t salen){
    int n;
    n = connect(fd, sa, salen);
    if(n<0)
        perr_exit("connect error");
    return n;
}
int Close(int fd){
    int n;
    if((n=close(fd))<0)
        perr_exit("close error");
    return n;
}
ssize_t Read(int fd, char *ptr, size_t nbyte){
    ssize_t n;
    do{
        n = read(fd, ptr, nbyte);
        if(n>=0)
            return n;
    }while(errno==EINTR);
    perr_exit("read error");
    return -1;
}
ssize_t Write(int fd, const void* ptr, size_t nbytes){
    ssize_t n;
    do{
        n = write(fd, ptr, nbytes);
        if(n>=0)
            return n;
    }while(errno==EINTR);
    perr_exit("write error");
    return -1;
}
int main(){
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    char buf[BUFSIZ];
    // 1. 创建通信的套接字
    int cfd = Socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1)
    {
        perror("socket");
        exit(0);
    }
    bzero(&serv_addr,sizeof(serv_addr));
    // 2. 连接服务器
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(SERV_PORT);
    int ret = Connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(ret == -1)
    {
        perror("connect");
        exit(0);
    }
    // 3. 和服务器端通信
    //while(1){
        /*fgets(buf, sizeof(buf), stdin);
        Write(cfd, buf, strlen(buf));
        ssize_t n = Read(cfd, buf, sizeof(buf));
        Write(STDOUT_FILENO, buf, n);*/
        // 发送数据
        //json -> buf
        json req;
        req["type"]=DISTRIBUTION_REQUEST;
        std::string str_out = req.dump();
        //send
        Write(cfd, (char *)str_out.data(), str_out.length());
        // 接收数据
        memset(buf, 0, sizeof(buf));
        socklen_t len = Read(cfd, buf, sizeof(buf));
        if(len > 0)
        {
            printf("服务器say: %s\n", buf);
        }
        else if(len  == 0)
        {
            printf("服务器断开了连接...\n");
            //break;
        }
        else
        {
            perror("read");
            //break;
        }
    //}
    Close(cfd);
    return 0;
}


