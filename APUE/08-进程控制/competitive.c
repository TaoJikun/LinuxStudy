#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void charactatime(char *str);


int main()
{
    __pid_t pid;
    if ((pid = fork()) < 0)
    {
        printf("fork() error!\n");
    }
    else if(pid == 0)
    {
        charactatime("output from child!\n");
    }
    else
    {
        charactatime("output from parent!\n");
    }
    exit(0);
}


void charactatime(char *str)
{
    char *ptr;
    int c;

    setbuf(stdout,NULL);//将标准输出设置为无缓冲的，使得每一次输出都要进行I/O
    for(ptr = str;*ptr!=0;++ptr)
    {
        c=*ptr;
        putc(c,stdout);
    }
}
