#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(void)
{
    struct stat statbuf;

    //获取"foo"文件的stat
    if(stat("foo",&statbuf) < 0){
        puts("stat() error!");
        exit(1);
    }
    
    //打开"foo"文件的执行时设置用户ID模式，关闭组执行模式
    if(chmod("foo",(statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0){
        puts("chmod() error!");
        exit(1);
    }

    //将"bar"文件设置为"rw-r--r--"模式
    if(chmod("bar", S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0){
        puts("chmod() error!");
        exit(1);
    }

    exit(0);
}