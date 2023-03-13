#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define RWRWRW S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

int main(void)
{
    umask(0);//无屏蔽字
    if(creat("foo",RWRWRW) < 0){
        puts("creat() error!");
    }
    umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);//屏蔽所有组和其他用户的访问权限
    if(creat("bar",RWRWRW) < 0){
        puts("creat() error!");
    }

    exit(0);
}