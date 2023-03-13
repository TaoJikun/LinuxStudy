[TOC]


# 1.1 引言



# 1.2 Unix体系结构

内核的接口被称为**系统调用**。
公用函数库构建在系统调用接口之上，应用程序既可使用公用函数库,也可使用系统调用。
shell是一个特殊的应用程序，为运行其他应用程序提供了一个接口。



# 1.3 登录



# 1.4 文件和目录

## 文件系统
UNIX文件系统是目录和文件的一种层次结构，所有东西的起点是称为根(root)的目录，这个目录的名称是一个字符"/"。

目录是一个包含目录项的文件。在逻辑上，可以认为每个目录项都包含一个文件名，同时还包含说明该文件属性的信息。文件属性是指文件类型(是普通文件还是目录等)、文件大小、文件所有者、文件权限(其他用户能否访问该文件)以及文件最后的修改时间等。stat和fstat函数返回包含所有文件属性的一个信息结构。


## 文件名
目录中的各个名字称为文件名。只有斜线(/)和空字符这两个字符不能出现在文件名中。斜线用来分隔构成路径名的各文件名，空字符则用来终止一个路径名。

创建新目录时会自动创建了两个文件名:.(称为点)和..(称为点点)。点指向当前目录，点点指向父目录。在最高层次的根目录中，点点与点相同。


## 路径名
由斜线分隔的一个或多个文件名组成的序列(也可以斜线开头)构成路径名，以斜线开头的路径名称为绝对路径名，否则称为相对路径名。

相对路径名指向相对于当前目录的文件。文件系统根的名字(/)是一个特殊的绝对路径名，它不包含文件名。

代码示例
```C++
//实现ls指令
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char* argv[])
{
    DIR *dp;//目录流对象的数据类型
    struct dirent *dirp;

    if (argc != 2)
    {
        printf("usage: ls direction_name!\n");
        exit(1);
    }

    if((dp = opendir(argv[1])) == NULL)//打开输入所对应的路径，返回一个DIR指针
    {
        printf("can't open %s \n",argv[1]);
        exit(1);
    }
    
    while((dirp = readdir(dp)) != NULL)
        printf("%s \n",dirp->d_name);

    closedir(dp);
    exit(0);
    
}
```


## 工作目录
每个进程都有一个工作目录，有时称其为当前工作目录。所有相对路径名都从工作目录开始解释。进程可以用`chdir`函数更改其工作目录。


## 起始目录
登录时，工作目录设置为起始目录，该起始目录从口令文件中相应用户的登录项中取得。



# 1.5 输入和输出

## 文件描述符
文件描述符通常是一个小的非负整数，内核用以标识一个特定进程正在访问的文件。
当内核打开一个现有文件或创建一个新文件时，它都返回一个文件描述符。
在读、写文件时，可以使用这个文件描述符。


## 标准输入、标准输出、标准错误
每当运行一个新程序时，所有的shell都为其打开3个文件描述符，即标准输入(`STDIN_FILENO`，值为0)、标准输出(`STDOUT_FILENO`，值为1)以及标准错误(`STDERR_FILENO`，值为2)。如果不做特殊处理，这三个描述符都链接向终端。


## 不带缓冲的I/O
函数`open、 read、 write、lseek、close`提供了不带缓冲的I/O。这些函数都使用文件描述符。

代码示例
```C++
//从标准输入读入数据，再写到标准输出中
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    int n;
    char buf[BUF_SIZE];

    //这里使用的read、write以及open、lseek、close都是不带缓冲的I/O
    //相反，标准I/O都是提供缓冲的
    while((n=read(STDIN_FILENO,buf,BUF_SIZE)) > 0)
    {
        buf[n]=0;
        if(write(STDOUT_FILENO,buf,n) != n){
            puts("write error!\n");
            exit(1);
        }
    }

    if(n<0){
        puts("read error!");
        exit(1);
    }

    exit(0);
}
```


## 标准I/O
标准I/O函数为那些不带缓冲的I/O函数提供了一个带缓冲的接口。
使用标准I/O函数无需担心如何选取最佳的缓冲区大小，
使用标准I/O函数还简化了对输入行的处理。



# 1.6 程序和进程

## 程序
程序是一个存储在磁盘上某个目录中的**可执行文件**。内核使用`exec`函数(7个exec函数之一)，将程序读入内存，并执行程序。


## 进程和进程ID
程序的执行实例被称为进程。
UNIX系统确保每个进程都有一个唯一的数字标识符，称为进程ID。进程ID总是一个非负整数。
调用函数`getpid`得到当前进程的进程ID。


## 进程控制
有3个用于进程控制的主要函数：`fork、exec、waitpid`。(exec函数有7种变体，但经常把它们统称为exec函数族)

代码示例
```C++
//从标准输入读命令并由子进程执行
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAXLINE 4096

int main(int argc, char *argv[])
{
    char buf[MAXLINE];
    pid_t pid;
    int status;

    printf("%% "); //打印print("%%")两个%%才能达到输出"%"的目的
	
    while (fgets(buf, MAXLINE, stdin) != NULL) 
    {
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0; //将输入的指令字符串的最后一位的换行符改为0(NULL)，因为execpl函数需要求参数以null结尾

        if ((pid = fork()) < 0) 
        {
            printf("fork error!\n");
            exit(1);
        } 
        else if (pid == 0) {		/* child */
            execlp(buf, buf, (char *)0); //exce协议族函数
			
            printf("couldn't execute: %s \n", buf);//重新加载输入的指令函数失败，直接终止子进程
            exit(127);
        }

		/* parent */
        if ((pid = waitpid(pid, &status, 0)) < 0)//阻塞直至某个子进程终止，并返回子进程的终止状态
        {
            printf("waitpid error!\n");
            exit(1);
        }
        printf("%% ");
    }
    
    exit(0);
}
```


## 线程和线程ID
一个进程内的所有线程共享同一地址空间、文件描述符、栈以及与进程相关的属性。

由于统一进程的多个线程能访问同一存储区，所以各线程在访问共享数据时需要采取同步措施以避免不一致性。

与进程相同，线程也用ID标识。但是，线程ID只在它所属的进程内起作用。一个进程中的线程ID在另一个进程中没有意义。



# 1.7 出错处理
当UNIX系统函数出错时，通常会返回一个负值，而且整型变量`errno`通常被设置为具有特定信息的值。

在支持线程的环境中，多个线程共享进程地址空间，每个线程都有属于它自己的局部`errno`以避免一个线程干扰另一个线程。

Linux支持多线程存取errno，将其定义为:
```C++
#include <errno.h>
extern int *__errno_location (void);
# define errno (*__errno_location ())
```

对于errno应当注意两条规则：
* 1、如果没有出错，其值不会被例程清除。因此，仅当函数的返回值指明出错时，才检验其值。
* 2、任何函数都不会将errno值设置为0，而且在<errno.h>中定义的所有常量都不为0。

C标准定义了两个函数用于打印`errno`的错误信息：
```C++
#include <string.h>
extern char *strerror (int __errnum);//返回指向消息字符串的指针
```
`strerror`函数将__errnum(通常就是errno值)映射为一个出错消息字符串，并且返回此字符串的指针。

```C++
#include <stdio.h>
extern void perror (const char *__s);//返回指向消息字符串的指针
```
`perror`函数基于errno的当前值，在标准错误上产生一条出错消息，然后返回。它首先输出由msg 指向的字符串，然后是一个冒号，一个空格，接着是对应于errno值的出错消息，最后是一个换行符。


示例代码
```C++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    //#define	ENOENT 	 2	/* No such file or directory */
    //#define	EACCES 	13	/* Permission denied */
    fprintf(stderr, "EACCES: %s\n", strerror(EACCES));
    errno = ENOENT;
    perror(argv[0]);
    exit(0);
}
```

运行结果
> EACCES: Permission denied
> ./testerrno: No such file or directory


出错恢复：
* 致命性错误：无法执行恢复动作。最多能做的是在用户屏幕上打印出一条出错消息或者将一条出错消息写入日志文件中，然后退出。
* 非致命性错误：有时可以较妥善地进行处理。大多数非致命性出错是暂时的(如资源短缺)，当系统中的活动较少时，这种出错很可能不会发生。对于资源相关的非致命性出错的典型恢复操作是延迟一段时间，然后重试。



# 1.8 用户标识

## 用户ID
口令文件登录项中的用户ID是一个数值，它向系统标识各个不同的用户。
系统管理员在确定一个用户的登录名的同时，确定其用户ID。
用户不能更改其用户ID。通常每个用户有一个唯一的用户ID。
用户ID为0的用户为根用户或超级用户。
调用函数`getuid`得到当前的用户ID。

## 组ID
口令文件登录项也包括用户的组ID，它是一个数值。
组ID也是由系统管理员在指定用户登录名时分配的。一般来说，在口令文件中有多个登录项具有相同的组ID。
组被用于将若干用户集合到项目或部门中去。这种机制允许同组的各个成员之间共享资源(如文件)。
调用函数`getgid`得到组ID。



# 1.9 信号

信号(signal)用于通知进程发生了某种情况。

进程有以下3种处理信号的方式:
* 1、忽略信号；
* 2、按系统默认方式处理；
* 3、提供信号处理函数，信号发生时由系统调用该函数(捕捉该信号)。



# 1.10 时间值

* 日历时间：自协调世界时1970年1月1号00:00:00这个特定时间以来经过的秒数累计值，使用数据类型`time_t`保存这种时间值。
* 进程时间：也称为CPU时间，用以度量进程使用的中央处理器资源。进程时间以时钟滴答计算，使用数据类型`clock_t`保存这种时间值。

当度量一个进程的执行时间时，UNIX系统为一个进程维护了3个进程时间值:
* 1、时钟时间：称为墙上时钟时间，它是进程运行的时间总量，其值与系统中同时运行的进程数有关。
* 2、用户CPU时间：执行**用户指令**所用的时间量。
* 3、系统CPU时间：为该进程执行**内核**程序所经历的时间。



# 1.11 系统调用和库函数

所有的操作系统都提供多种服务的入口点，由此程序向内核请求服务。各种版本的UNIX实现都提供良好定义、数量有限、直接进入内核的入口点，这些入口点被称为**系统调用**。

系统调用和库函数之间的另一个差别是：系统调用通常提供一种最小接口，而库函数通常提供比较复杂的功能。
