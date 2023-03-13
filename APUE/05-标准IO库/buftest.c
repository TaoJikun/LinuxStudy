#include <stdio.h>
#include <stdlib.h>

void print_staio(const char* name, FILE *fp);
int is_unbuffered(FILE *fp);
int is_linebuffered(FILE *fp);
int buffer_size(FILE *fp);


int main(void)
{
    FILE *fp;

    fputs("enter any character: \n", stdout);

    if(getchar() == EOF){
        fputs("getchar() error!\n", stdin);
        exit(1);
    }

    fputs("one line to stderr!\n", stderr);

    print_staio("stdin", stdin);
    print_staio("stdout", stdout);
    print_staio("stderr", stderr);

    if((fp = fopen("/etc/passwd", "r")) == NULL){
        fputs("fopen() error!\n", stdout);
        exit(1);
    }

    if(getc(fp) == EOF){
        fputs("getc() error!\n", stdout);
        exit(1);
    }

    print_staio("etc/passwd", fp);
    exit(0);
}


void print_staio(const char* name, FILE *fp)
{
    printf("stream = %s, ", name);
    if(is_unbuffered(fp))
    {
        //无缓冲
        printf("unbuffered");
    }
    else if(is_linebuffered(fp))
    {
        //行缓冲
        printf("line buffered");
    }
    else
    {
        //全缓冲
        printf("fully buffered");
    }

    printf(", buffer size = %d \n", buffer_size(fp));
}



#if defined(_IO_UNBUFFERED)

int is_unbuffered(FILE *fp)
{
    return (fp->_flags & _IO_UNBUFFERED);    
}

int is_linebuffered(FILE *fp)
{
    return (fp->_flags & _IO_LINE_BUF);
}

int buffer_size(FILE *fp)
{
    //end of reserve area - start of reserve area
    return (fp->_IO_buf_end - fp->_IO_buf_base);
}

#elif defined(__SNBF)

int is_unbuffered(FILE *fp)
{
    return (fp->flags & __SNBF);
}

int is_linebuffered(FILE *fp)
{
    return (fp->flags & __SLBF);
}

int buffer_size(FILE *fp)
{
    return (fp->_bf._size);
}

#elif defined(_IONBF)

#ifdef _LP64
#define _flag __pad[4]
#define _ptr __pad[1]
#define _base __pad[2]
#endif

int is_unbuffered(FILE *fp)
{
    return (fp->flags & _IONBF);
}

int is_linebuffered(FILE *fp)
{
    return (fp->flags & _IOLBF);
}

int buffer_size(FILE *fp)
{
#ifdef _LP64
    return (fp->_base - fp->ptr);
#else
    return (BUFSIZ);
#endif
}

#else

#error unknown stdio implementation!

#endif
