#include <stdio.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
    int cfd1,cfd2;
    char str1[] = "Hi~\n";
    char str2[] = "It's a nice day~\n";

    cfd1 = dup(1);//复制标准输出的文件描述符
    cfd2 = dup2(cfd1,7);//复制cdf1的文件描述符，并将副本文件描述符整数值指定为7

    printf("fd1=%d, fd2=%d \n",cfd1,cfd2);
    write(cfd1,str1,sizeof(str1));
    write(cfd2,str2,sizeof(str2));

    close(cfd1);
    close(cfd2);
    //两个副本都被关闭了，但是原始的文件描述符还存在，故依旧能写
    write(1,str1,sizeof(str1));
    close(1);
    //原始的文件描述符也被关闭了，故不能在写了
    write(1,str2,sizeof(str2));

    return 0;
}