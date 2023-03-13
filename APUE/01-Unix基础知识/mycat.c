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