#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    fd_set readset,tempset;
    int result,str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;
    FD_ZERO(&readset);
    FD_SET(0,&readset);//标准IO的输入的描述符为0


    while(1)
    {
        tempset = readset;
        timeout.tv_sec=5;
        timeout.tv_usec=0;
        result = select(1,&tempset,NULL,NULL,&timeout);
        if(result == -1)
        {
            //select失败
            puts("select() error!");
            break;
        }
        else if(result == 0)
        {
            //超时
            puts("time out!");
        }
        else
        {
            if(FD_ISSET(0,&tempset))
            {
                //监视的描述符已就绪(标准IO输入已经准备好了数据)
                str_len = read(0,buf,BUF_SIZE);
                buf[str_len]=0;
                printf("message from console: %s \n",buf);
            }
        }
    }

    return 0;
}