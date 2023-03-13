#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 30

void error_handling(char * message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in recv_addr;
    if(argc!=3){
        printf("Usage: %s <IP> <Port> \n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET,SOCK_STREAM,0);
    memset(&recv_addr,0,sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    recv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock,(struct sockaddr*)&recv_addr,sizeof(recv_addr)) == -1)
        error_handling("connect() error!");

    write(sock,"123",strlen("123"));
    sleep(2);
    send(sock,"4",strlen("4"),MSG_OOB);
    sleep(2);
    write(sock,"567",strlen("567"));
    sleep(2);
    send(sock,"890",strlen("890"),MSG_OOB);

    close(sock);
    return 0;
}

void error_handling(char * message)
{
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}