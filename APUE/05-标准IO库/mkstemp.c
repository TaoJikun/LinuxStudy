#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>


void make_temp(char *template);


int main(void)
{
    char good_template[] = "/tmp/dirXXXXXX";//将字符串常量拷贝到了栈中
    char *bad_template = "/tmp/dirXXXXXX";//bad_template指针指向在全局数据区的字符串常量，属于只读内存，不可修改末尾的6位XXXXXX占位符
    
    //下面这样拷贝一份字符串常量就可以了
    /*char *bad_template;
    bad_template = (char*)malloc(sizeof(char)*14);
    memcpy(bad_template,good_template,14);*/

    printf("trying to create first temp file...\n");
    make_temp(good_template);
    printf("trying to create second temp file...\n");
    make_temp(bad_template);
    exit(0);
}


void make_temp(char *template)
{
    int fd;
    struct stat sbuf;
    
    if((fd = mkstemp(template)) < 0)//创建临时文件
    {
        printf("mkstemp() error!\n");
    }

    printf("temp name = %s \n", template);

    close(fd);

    if(stat(template, &sbuf) < 0)//stat函数获取临时文件的状态，判断临时文件是否存在
    {
        if(errno == ENOENT){
            printf("file doesn't exist!\n");
        }
        else{
            printf("stat() error!\n");
        }
    }
    else{
        printf("file exists!\n");
        unlink(template);
    }
}