#include <iostream>
#include <string.h>
#include <unordered_map>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>


#define MAX_LEN 4096


// 读取服务器端推送的消息
void* ReadMsg(void* sockfd)
{
    int clientfd = *((int*)sockfd);

    char buffer[MAX_LEN];
    while(1)
    {
        scanf("%s", buffer);
        if(strcmp(buffer,"QUIT\n") == 0 || strcmp(buffer,"qiut\n"))
        {
            //当客户端输入quit或者QUIT时断开连接
            pthread_exit(0);
        }

        int ret = write(clientfd, buffer, strlen(buffer));
        if(ret < 0)
        {
            perror("Failed to write()!\n");
            pthread_exit(NULL);
        }
    }

    pthread_exit(0);
}

// 向服务器发送消息
void* WriteMsg(void* sockfd)
{
    int clientfd = *((int*)sockfd);

    char buffer[MAX_LEN];
    while(1)
    {
        int nbytes = read(clientfd, buffer, MAX_LEN-1);
        if(nbytes < 0)
        {
            perror("Failed to read()!\n");
            pthread_exit(NULL);
        }
        
        buffer[nbytes] = '\0';
        printf("%s", buffer);
    }

    pthread_exit(0);
}


int main(int argc, char* argv[])
{
    int ret = 0;

    if(argc < 3)
    {
        perror("Input Error!\n");
        exit(-1);
    }

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd < 0)
    {
        perror("Failed to socket()!\n");
        exit(-1);
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    ret = inet_pton(AF_INET, argv[1], (void*)&serverAddr.sin_addr);
    if(ret != 1)
    {
        perror("Failed to inet_pton()!\n");
        exit(-1);
    }

    //ret = inet_pton(AF_INET, argv[2], (void*)&serverAddr.sin_port);
    if(ret != 1)
    {
        perror("Failed to inet_pton()!\n");
        exit(-1);
    }

    ret = connect(clientfd, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret != 0)
    {
        perror("Failed to connect()!\n");
        exit(-1);
    }

    printf("连接服务器成功，请输入用户名：\n");
    
    char name[MAX_LEN];
    scanf("%s",name);

    //读写分离，使用两个线程分别向服务器发送消息和接收服务器推送的消息
    pthread_t readThread,writeThread;
    ret = pthread_create(&readThread, NULL, ReadMsg, (void*)&clientfd);
    if(ret != 0)
    {
        perror("Failed to pthread_create()!\n");
        exit(-1);
    }

    ret = pthread_create(&writeThread, NULL, WriteMsg, (void*)&clientfd);
    if(ret != 0)
    {
        perror("Failed to pthread_create()!\n");
        exit(-1);
    }

    ret = pthread_join(readThread, NULL);
    if(ret != 0)
    {
        perror("Failed to pthread_join()!\n");
        exit(-1);
    }

    ret = pthread_join(writeThread, NULL);
    if(ret != 0)
    {
        perror("Failed to pthread_join()!\n");
        exit(-1);
    }


    close(clientfd);

    exit(0);
}