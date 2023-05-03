#include <iostream>
#include <string.h>
#include <unordered_map>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CONNECT 4096 //最大连接数

#define MAX_MESSAGE_LEN 4096 //一次接收的最大字节数

#define LOCAL_IP_ADDR '123.60.151.203'
#define LOCAL_IP_PORT 8080


// 客户端的信息
class Client{
public:
    int sockfd;         //客户端使用的套接字
    std::string name;   //客户端的名字
};



int main()
{
    int ret = 0;

    std::unordered_map<int, Client> clients;

    //创建epoll
    int epollfd = epoll_create(MAX_CONNECT);
    if(epollfd < 0)
    {
        perror("Failed to epoll_create()!\n");
        exit(-1);
    }

    //创建监听套接字
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd < 0)
    {
        perror("Failed to socket()!\n");
        exit(-1);
    }


    //将监听套接字绑定到指定的IP+端口
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(LOCAL_IP_PORT);
    ret = bind(listenfd, (sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        perror("Failed to bind()!\n");
        exit(-1);
    }

    //监听客户端的连接
    ret = listen(listenfd, 5); //backlog设为5，表示半连接队列和全连接队列的相关值
    if(ret < 0)
    {
        perror("Failed to listen()!\n");
        exit(-1);
    }

    //将监听套接字添加到epoll中
    epoll_event addevent;
    addevent.events = EPOLLIN; //发生需要读取数据的情况时
    addevent.data.fd = listenfd;
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &addevent);
    if(ret < 0)
    {
        perror("Failed to epoll_ctl()!\n");
        exit(-1);
    }


    //循环监听请求连接的客户端
    epoll_event events[MAX_CONNECT+1];
    while(1)
    {
        int count = epoll_wait(epollfd, events, MAX_CONNECT+1, -1);
        if(count < 0)
        {
            perror("Failed to epoll_wait()!\n");
            exit(-1);
        }

        for(int i=0;i<count;++i)
        {
            int fd = events[i].data.fd;
            if(fd == listenfd)
            {
                //有客户端连接到来
                sockaddr_in clientAddr;
                memset(&clientAddr, 0, sizeof(clientAddr));
                socklen_t len = sizeof(clientAddr);
                int clientfd = accept(listenfd, (sockaddr*)&clientAddr, &len);
                if(clientfd < 0)
                {
                    perror("Failed to accept()!\n");
                    continue;
                    //exit(-1);
                }

                //将客户端连接对应的套接字加入epoll中
                epoll_event clientEvent;
                clientEvent.events = EPOLLIN; //发生需要读取数据的情况时
                clientEvent.data.fd = clientfd;
                ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &clientEvent);
                if(ret < 0)
                {
                    perror("Failed to epoll_ctl()!\n");
                    exit(-1);
                }

                char peerIP[16];
                if(!inet_ntop(AF_INET, (void*)&clientAddr.sin_addr.s_addr, peerIP, (socklen_t)sizeof(clientAddr.sin_addr.s_addr)))
                {
                    perror("Failed to inet_ntop()!\n");
                    exit(-1);
                }
                //printf("%s已连接...\n", clientAddr.sin_addr.s_addr);
                printf("%s已连接...\n", peerIP);
                
                //保存已连接用户的信息
                Client connectedClient;
                connectedClient.sockfd = clientfd;
                connectedClient.name = "";
                clients[clientfd] = connectedClient;
            }
            else
            {
                //有读事件就绪
                char buffer[MAX_MESSAGE_LEN];
                int nbytes = read(fd, buffer, MAX_MESSAGE_LEN);
                if(nbytes < 0)
                {
                    perror("Failed to read()!\n");
                    exit(-1);
                }
                else if(nbytes == 0)
                {
                    //EOF
                    epoll_event removeevent;
                    removeevent.events = EPOLLIN;
                    removeevent.data.fd = fd;
                    ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &removeevent);
                    if(ret < 0)
                    {
                        perror("Failed to epoll_ctl()!\n");
                        exit(-1);
                    }

                    clients.erase(fd); //删除该客户信息
                    close(fd); //断开连接
                }
                else
                {
                    std::string msg(buffer);

                    if(clients[fd].name == "")
                    {
                        //客户端先初始化其名称
                        clients[fd].name += '[';
                        clients[fd].name += msg;
                        clients[fd].name += ']';
                        continue;
                    }

                    //将客户端发来的消息推送给其他用户
                    std::string sendMsg = clients[fd].name;
                    sendMsg += ": ";
                    sendMsg += msg;
                    for(auto itr = clients.begin(); itr!=clients.end(); ++itr)
                    {
                        if(itr->first != fd)
                        {
                            ret = write(itr->first, sendMsg.c_str(), sendMsg.size());
                            if(ret < 0)
                            {
                                perror("Failed to write()!\n");
                                exit(-1);
                            }
                        }
                    }
                }
            }
        }
    }

    close(listenfd);
    close(epollfd);

    exit(0);
}