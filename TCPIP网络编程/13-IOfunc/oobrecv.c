#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define BUF_SIZE 30

void error_handling(char * message);
void urg_handler(int singno);

int listen_sock;
int recv_sock;

int main(int argc, char *argv[])
{
    struct sockaddr_in recv_addr,serv_addr;
    int str_len,state;
    socklen_t recv_addr_len;
    struct sigaction act;
    char buf[BUF_SIZE];
     if(argc!=2){
        printf("Usage: %s <Port> \n", argv[0]);
        exit(1);
    }

    act.sa_handler = urg_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;

    listen_sock = socket(PF_INET,SOCK_STREAM,0);
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(listen_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1)
        error_handling("bind() error!");
    
    listen(listen_sock,5);

    recv_addr_len = sizeof(recv_addr);

    recv_sock = accept(listen_sock,(struct sockaddr*)&recv_addr,&recv_addr_len);

    fcntl(recv_sock,F_SETOWN,getpid());//将连接套接字的属主设为本进程
    state = sigaction(SIGURG,&act,0);//捕获OBB外带数据的信号

    while(str_len = recv(recv_sock,buf,sizeof(buf),0) != 0){
        if(str_len == -1)
        {
            continue;
        }
        buf[BUF_SIZE]=0;
        puts(buf);
    }
    
    close(recv_sock);
    close(listen_sock);
    return 0;
}

void error_handling(char * message)
{
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

void urg_handler(int signo)
{
    int str_len;
    char buf[BUF_SIZE];
    str_len = recv(recv_sock,buf,sizeof(buf)-1,MSG_OOB);
    buf[str_len]=0;
    printf("Urgent Message: %s \n",buf);
}