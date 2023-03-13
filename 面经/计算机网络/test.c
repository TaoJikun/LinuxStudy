#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

int main(){
    int fd1,fd2;
    fd1 = socket(AF_INET,SOCK_STREAM,0);
    fd2 = socket(AF_INET,SOCK_STREAM,0);
    
    int val=1;
    setsockopt(fd1,SOL_SOCKET,SO_REUSEADDR,(void*)&val,(socklen_t)sizeof(val));
    setsockopt(fd2,SOL_SOCKET,SO_REUSEADDR,(void*)&val,(socklen_t)sizeof(val));


    struct sockaddr_in addr1,addr2;
    
    memset(&addr1,0,sizeof(addr1));
    addr1.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &addr1.sin_addr);
    addr1.sin_port = htons(8888);

    memset(&addr2,0,sizeof(addr2));
    addr2.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr2.sin_addr);
    addr2.sin_port = htons(8888);
    
    int tag1 = bind(fd1,(struct sockaddr*)&addr1,(socklen_t)sizeof(addr1));
    if(tag1<0){
        printf("fail1!\n");
    }
    int tag2 = bind(fd2,(struct sockaddr*)&addr2,(socklen_t)sizeof(addr2));
    if(tag2<0){
        printf("fail2!\n");
    }

}