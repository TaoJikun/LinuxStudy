#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <aio.h>
#include <sys/stat.h>

#define BUFSZ 4096
#define NBUF 8
#define FILEMODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

enum rwop{
    UNUSED = 0,
    READ_PENDING = 1,
    WRITE_PENDING = 2
};

struct buf{
    enum rwop op;
    int last;
    struct aiocb aiocb;
    unsigned char data[BUFSIZ];
};

struct buf bufs[NBUF];

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
    int ifd, ofd, i, j, n, err, numop;
    struct stat sbuf;
    const struct aiocb *aiolist[NBUF];
    off_t off = 0;

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
    if(fstat(ifd,&sbuf) < 0)
    {
        printf("stat failed!\n");
        exit(0);
    }

    for(i=0;i<NBUF;++i)
    {
        bufs[i].op = UNUSED;
        bufs[i].aiocb.aio_buf = bufs[i].data;
        bufs[i].aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
        aiolist[i] = NULL;
    }

    numop = 0;
    for(;;)
    {
        for(i=0;i<NBUF;++i)
        {
            switch (bufs[i].op)
            {
            case UNUSED:
                if(off < sbuf.st_size)
                {
                    bufs[i].op = READ_PENDING;
                    bufs[i].aiocb.aio_fildes = ifd;
                    bufs[i].aiocb.aio_offset = off;
                    off += BUFSZ;
                    if(off >= sbuf.st_size)
                        bufs[i].last = i;
                    bufs[i].aiocb.aio_nbytes = BUFSZ;
                    if(aio_read(&bufs[i].aiocb) < 0)
                    {
                        printf("aio_read failed!\n");
                        exit(0);
                    }
                    aiolist[i] = &bufs[i].aiocb;
                    numop++;
                }
                break;
            
            case READ_PENDING:
                if((err = aio_error(&bufs[i].aiocb)) == EINPROGRESS)
                    continue;
                if(err != 0)
                {    
                    if(err == -1)
                        printf("aio_error failed!\n");
                    else
                    {
                        printf("%d read failed!\n", err);
                        exit(0);
                    }
                }
                if((n = aio_return(&bufs[i].aiocb)) < 0)
                {
                    printf("aio_return failed!\n");
                    exit(0);
                }
                if(n != BUFSZ && !bufs[i].last)
                    printf("short read (%d/%d)!\n", n, BUFSZ);
                for(j = 0; j < n; j++)
                    bufs[i].data[j] = translate(bufs[i].data[j]);
                bufs[i].op = WRITE_PENDING;
                bufs[i].aiocb.aio_fildes = ofd;
                bufs[i].aiocb.aio_nbytes = n;
                if(aio_write(&bufs[i].aiocb) < 0)
                {
                    printf("aio_write failed!\n");
                    exit(0);
                }
                break;
            
            case WRITE_PENDING:
                if((err = aio_error(&bufs[i].aiocb)) == EINPROGRESS)
                    continue;
                if(err != 0)    
                {
                    if(err == -1)
                        printf("aio_error failed!\n");
                    else
                    {
                        printf("%d write failed!\n", err);
                        exit(0);
                    }
                }
                
                if((n = aio_return(&bufs[i].aiocb)) < 0)
                    printf("aio_return failed!\n");
                if(n != bufs[i].aiocb.aio_nbytes)
                    printf("short write (%d/%d)!\n", n, BUFSZ);
                aiolist[i] = NULL;
                bufs[i].op = UNUSED;
                numop--;
                break;    
            
            default:
                break;
            }
        }

        if(numop == 0)
        {
            if(off >= sbuf.st_size)
                break;
        }
        else    
        {
            if(aio_suspend(aiolist, NBUF, NULL) < 0)
                printf("aio_suspend failed!\n");
        }
    }

    bufs[0].aiocb.aio_fildes = ofd;
    if(aio_fsync(O_SYNC, &bufs[0].aiocb) < 0)
        printf("aio_fsync failed!\n");
    exit(0);
}