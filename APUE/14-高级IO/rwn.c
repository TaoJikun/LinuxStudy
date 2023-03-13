#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

ssize_t readn(int fd, void *ptr, size_t n)
{
    size_t nleft;
    ssize_t nread;

    nleft = n;
    while(nleft>0)
    {
        if((nread = read(fd, ptr, nleft)) < 0)
        {
            if(nleft == n)
            {
                return(-1);//出错
            }
            else
            {
                break;//出错，终止循环，返回已经读过的字节数
            }
        }
        else if(nread == 0)
        {
            break;//EOF
        }
        nleft -= nread;
        ptr += nread;
    }

    return (n-nleft);
}



ssize_t writen(int fd, void *ptr, size_t n)
{
    size_t nleft;
    ssize_t nwriteten;

    nleft = n;
    while(nleft > 0)
    {
        if((nwriteten = write(fd, ptr, n)) < 0)
        {
            if(nleft == n)
            {
                return(-1);//出错
            }
            else
            {
                break;//出错，终止循环，返回已经写过的字节数
            }
        }
        else if(nwriteten == 0)
        {
            break;
        }
        nleft -= nwriteten;
        ptr += nwriteten;
    }
    return (n-nleft);
}