#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/resource.h>

void daemonize(const char* cmd)
{
    int i,fd0,fd1,fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    //调用umask将文件模式创建屏蔽字设置为0
    umask(0);

    //获取文件描述符的最大数量
    if(getrlimit(RLIMIT_NOFILE,&rl) < 0)
    {
        printf("%s can't get file limit!\n",cmd);
    }

    //成为一个新会话的首进程，并且脱离控制终端
    if((pid = fork()) < 0)
    {
        printf("%s can't fork()\n",cmd);
    }
    else if(pid != 0)
    {
        exit(0);
    }

    //调用setsid创建一个新的会话
    setsid();

    //确保后续的操作不会allocate控制终端
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0)
    {
        printf("%s can't ignaor SIGHUP\n",cmd);
    }
    if((pid = fork()) < 0)
    {
        printf("%s can't fork()\n",cmd);
    }
    else if(pid != 0)
    {
        exit(0);
    }

    //将当前工作目录更改为根目录，这样就不会阻止卸载文件系统。
    if(chdir("/") < 0)
    {
        printf("%s can't change directory to /\n",cmd);
    }

    //关闭所有的文件描述符
    if(rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for(i=0;i<rl.rlim_max;++i)
    {
        close(i);
    }

    //将文件描述符0，1，2关联到/dev/null
    fd0 = open("/dev/null",O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    //初始化log file
    openlog(cmd,LOG_CONS,LOG_DAEMON);
    if(fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0,fd1,fd2);
        exit(0);
    }
}


int main(int argc, char *argv[])
{
    daemonize(argv[1]);
    sleep(10);
}