#include "iostream"
#include "unistd.h"
#include "cstring"
#include "arpa/inet.h"
#include "sys/epoll.h"
#include "cerrno"
#include "cctype"
#include "fcntl.h"
#include "json.hpp"
#include "signal.h"
using namespace std;
using json = nlohmann::json;
#define MAXLINE 8192
#define SERV_PORT 7000
#define OPEN_MAX 5000
#define SHUTDOWN_CACHE 6
#define ADD_CACHE 7
ssize_t unblock_read_net(int cfd, char *buf, int size){
    std::string res="";
    bool flag=false;
    ssize_t rtn = 0;
    int n=0;
    do{
        n = read(cfd, buf, size);
        if(n>0&&!flag){
            for(int i=0; i<n; i++){
                if(buf[i]=='\0'){
                    flag=true;
                }
                if(!flag){
                    res += buf[i];
                    rtn++;
                }
            }
        }
    }while(n>0 || errno==EINTR);
    if(rtn==0)
        return n;
    int i=0;
    for(char c:res){
        buf[i++] = c;
    }
    buf[i] = '\0';
    return rtn;
}

int main(){
    int num = 0, listen_num = 20;
    char buf[100], str[INET_ADDRSTRLEN];

    struct sockaddr_in cliaddr, servaddr;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, listen_num);
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    auto flags = fcntl(connfd,F_GETFL,0);
    fcntl(connfd,F_SETFL,flags|O_NONBLOCK);
    signal(SIGPIPE,SIG_IGN);
    while(1){
        auto n = unblock_read_net(connfd, buf, 100);
        if(n>=0){
            cout<<buf<<endl;
        }
        sleep(1);
    }
    close(listenfd);
    close(connfd);
    return 0;
}
