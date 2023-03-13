/*
    使用存储映射I/O复制文件
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define COPYNCR (1024*1024*1024) /* 1GB */
#define FILEMODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

int main(int argc, char *argv[])
{
    int fdin, fdout;
    void *src, *dst;
    size_t copysz;
    struct stat sbuf;
    off_t fsz = 0;

    if(argc != 3)
    {
        printf("usage: %s <fromfile> <tofile>!\n", argv[0]);
        exit(0);
    }

    if ((fdin = open(argv[1], O_RDONLY)) < 0)
    {
        printf("can't open %s fro reading!\n", argv[1]);
        exit(0);
    }

    if ((fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILEMODE)) < 0)
    {
        printf("can't open %s for writeing!\n", argv[2]);
        exit(0);
    }

    //获取输入文件的长度
    if(fstat(fdin, &sbuf) < 0)
    {
        printf("failed to fstat!\n");
        exit(0);
    }
    
    //将打开的文件fout截断为指定字节
    if(ftruncate(fdout, sbuf.st_size) < 0)
    {
        printf("failed to ftruncate!\n");
        exit(0);
    }

    while(fsz < sbuf.st_size)
    {
        //计算剩下要拷贝的字节数，如果大于1GB字节，则这次只拷贝1GB
        if(sbuf.st_size - fsz > COPYNCR)
        {
            copysz = COPYNCR;
        }
        else
        {
            copysz = sbuf.st_size - fsz;
        }

        //将输入/输出文件映射到存储空间的缓存区中
        if((src = mmap(0, copysz, PROT_READ, MAP_SHARED, fdin, fsz)) == MAP_FAILED)
        {
            printf("faield to mmap for input!\n");
            exit(0);
        }
        if((dst = mmap(0,copysz, PROT_WRITE|PROT_READ, MAP_SHARED, fdout, fsz)) == MAP_FAILED)
        {
            printf("faield to mmap for output!\n");
            exit(0);
        }

        //拷贝
        memcpy(dst, src, copysz);

        //解除映射
        munmap(src, copysz);
        munmap(dst, copysz);

        fsz += copysz;
    }

    exit(0);

}