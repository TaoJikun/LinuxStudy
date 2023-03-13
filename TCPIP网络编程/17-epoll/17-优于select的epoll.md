[TOC]

# 17.1 epoll理解及应用

## 基于select的I/O复用技术速度慢的原因
>* 调用select函数后要针对所有文件描述符进行`FD_ISSET`的循环语句。
>* 每次调用select函数时都需要向该函数传递监视对象信息。

上面两点中对于提升性能的更大障碍是**每次传递监视对象信息**（第二点）。传递监视对象信息实质上是每次调用select函数时向操作系统传递监视对象信息。应用程序向操作系统传递数据将对程序造成很大负担，而且无法通过优化代码解决，因此将成为性能上的致命弱点。


select函数需要向操作系统传递监视对象信息的原因：select函数与文件描述符有关，是监视套接字变化的函数，而套接字是由操作系统管理的，所以select函数绝对需要借助于操作系统才能完成功能。

select函数上述缺点的弥补方式：**仅向操作系统传递1次监视对象，监视范围或内容发生变化时只通知发生变化的事项**。Linux支持的方式是epoll，Windows支持的方式是IOCP，这就说明改进的I/O复用模型不具有兼容性，而select函数却能得到大部分操作系统的支持。


## epoll函数
epoll函数**优点**：
+ 无需编写以监视状态变化为目的的针对所有文件描述符的循环语句。
+ 调用对应于select函数的epoll_wait函数时无需每次传递监视对象信息。


epoll的三个函数：
+ epoll_creat：创建保存epoll文件描述符的空间。
+ epoll_ctl：向空间注册并注销文件描述符。
+ epoll_wait：与select函数类似,等待文件描述符发生变化。

select和epoll的**区别**：
> 1. select方式中为了保存监视对象文件描述符，直接声明了fd_set变量。但epoll方式下由操作系统负责保存监视对象文件描述符，因此需要向操作系统请求创建保存文件描述符的空间，此时使用的函数就是epoll_create。
> 2. 为了添加和删除监视对象文件描述符，select方式中需要FD_SET、FD_CLR函数。但在epoll方式中，通过epoll_ctl函数请求操作系统完成。
> 3. select方式下调用select函数等待文件描述符的变化，而epoll中调用epoll_wait函数。
> 4. select方式中通过fd_set变量查看监视对象的状态变化（事件发生与否），而epoll方式中通过结构体epoll_event将发生变化的文件描述符单独集中到一起。

epoll_event结构体：
```C++
struct epoll_event
{
    __unit32_t events;
    epoll_data_t data;
}

typedef union epoll_data
{
    void * ptr;
    int fd;
    __unit32_t u32;
    __unit64_t u64;
} epoll_data_t;
```
声明足够大的epoll_event结构体数组后，传递给epoll_wait函数时，发生变化的文件描述符信息将被填入该数组。因此，无需像select函数那样针对所有文件描述符进行循环。


### epoll_create
```C++
#include <sys/epoll.h>
int epoll_create(int size);//成功时返回epol1文件描述符,失败时返回-1。
```
调用epoll_create函数时创建的文件描述符保存空间称为“epoll例程”，通过参数size传递的值决定epoll例程的大小，但该值只是向操作系统提的建议。换言之，size并非用来决定epoll例程的大小，而仅供操作系统参考。（Linux 2.6.8之后的内核将完全忽略传入epoll_create函数的size参数，因为内核会根据情况调整epoll例程的大小）


### epoll_ctl
```C++
#include <sys/epoll.h>
int epoll_ctl(int epfd, int op, int fd, struct epoll_event * event);//成功时返回0,失败时返回-1。
//epfd————用于注册监视对象的epoll例程的文件描述符。
//op————用于指定监视对象的添加、删除或更改等操作。
//fd————需要注册的监视对象文件描述符。
//event————监视对象的事件类型。
```

调用形式：
```C++
//epoll例程A中注册文件描述符B，主要目的是监视参数C中的事件。
epoll_ctl(A, EPOLL_CTL_ADD, B, C);

//从epoll例程A中删除文件描述符B。
epoll_ctl(A, EPOLL_CTL_DEL, B, NULL);
```

第二个参数传递的常量及意义：
> EPOLL_CTL_ADD：将文件描述符注册到epoll例程。
> EPOLL_CTL_DEL：从epoll例程中删除文件描述符。
> EPOLL_CTL_MOD：更改注册的文件描述符的关注事件发生情况。


第四个参数的意义
```C++
//将sockfd注册到epoll例程epfd中，并在需要读取数据的情况下产生相应事件。
struct epoll_event event;
event.events = EPOLLIN;//发生需要读取数据的情况时
event.data.fd = sockfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
```

epoll_event的成员events中可以保存的常量及其所指事件类型：
> * EPOLLIN：需要读取数据的情况。
> * EPOLLOUT：输出缓冲为空，可以立即发送数据的情况。
> * EPOLLPRI：收到OOB数据的情况。
> * EPOLLRDHUP：断开连接或半关闭的情况，这在边缘触发方式下非常有用。
> * EPOLLERR：发生错误的情况。
> * EPOLLET：以边缘触发的方式得到事件通知。
> * EPOLLONESHOT：发生一次事件后，相应文件描述符不再收到事件通知。因此需要向epoll_ctl函数的第二个参数传递EPOLL_CTL_MOD，再次设置事件。


### epoll_wait
```C++
#include <sys/epoll.h>
int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);//成功时返回发生事件的文件描述符数目,失败时返回-1。
//epfd————表示事件发生监视范围的epoll例程的文件描述符。
//events————保存发生事件的文件描述符集合的结构体地址值，所指缓冲需要动态分配。
//maxevents————第二个参数中可以保存的最大事件数。
//timeout————以1/1000秒为单位的等待时间,传递-1时，一直等待直到发生事件。
```

调用形式
```C++
int event_cnt;
struct epoll_event * ep_events;
ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);//EPOLL_SIZE是宏常量
event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
```



# 17.2 条件触发和边缘触发

## 条件触发和边缘触发的区别在于发生事件的时间点
* 条件触发方式中，只要输入缓冲中还剩有数据，就将以事件方式再次注册。
* 边缘触发中输人缓冲收到数据时仅注册1次该事件，即使输入缓冲中还留有数据，也不会再进行注册。

epoll默认以条件触发方式工作。修改为边缘触发方式：`event.events = EPOLLIN | EPOLLET`。


## 边缘触发的服务器端实现中必知的两点
* 通过全局变量`errno`验证错误原因(访问errno需要引入error.h头文件，每种函数发生错误时,保存到errno变量中的值都不同)。
* 为了完成非阻塞IO，更改套接字特性(使用`fcntl`函数修改)。


## 条件触发和边缘触发孰优孰劣
边缘触发方式下可以做到：可以分离接收数据和处理数据的时间点！
