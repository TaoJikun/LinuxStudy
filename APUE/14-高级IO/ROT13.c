#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

#define BUFSZ 4096
#define FILEMODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

unsigned char buf[BUFSZ];

unsigned char translate(unsigned char c)
{
    if(isalpha(c))
    {
        if(c >= 'n')
            c -= 13;
        else if(c >= 'a')
            c += 13;
        else if(c >= 'N')
            c -= 13;
        else
            c += 13;
    }

    return c;
}

int main(int argc, char* argv[])
{
    int ifd, ofd, i, n, nw;

    if(argc != 3)
    {
        printf("usage: rot13 infile outfile!\n");
        exit(0);
    }

    if ((ifd = open(argv[1], O_RDONLY)) < 0)
    {
        printf("can't open %s\n", argv[1]);
        exit(0);
    }

    if ((ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILEMODE)) < 0)
    {
        printf("can't open %s\n", argv[2]);
        exit(0);
    }

    while((n = read(ifd, buf, BUFSZ)) > 0)
    {
        for(i=0;i<n;++i)
        {
            buf[i] = translate(buf[i]);
        }

        if((nw = write(ofd, buf, n)) != n)
        {
            if(nw < 0)
            {
                printf("write failed!\n");
                exit(0);
            }
            else
            {
                printf("short write!\n");
            }
        }
    }

    fsync(ofd);//将文件ofd写入磁盘（延迟写）并等待写完毕后再返回
    exit(0);
}