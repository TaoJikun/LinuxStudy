#include <stdio.h>
#include <string.h>

#define BSZ 48

int main()
{
    FILE *fp;
    char buf[BSZ];
    
    memset(buf, 'a', BSZ-2);
    buf[BSZ-2] = '\0';
    buf[BSZ-1] = 'X';//将buf的前46个字符置为'a',buf[46]置为结束符'\0'
    if ((fp = fmemopen(buf, BSZ, "w+")) == NULL)//将buf创建为内存流，同时在缓冲区开始处放置null(结束符\0)，并返回内存流指针，
    {
        printf("fmemopen() failed!\n");
    }
    
    printf("initial buffer contents: %s\n", buf);
    fprintf(fp, "hello, world");//向内存流的缓冲区（非buf）中写入数据
    printf("before flush: %s\n", buf);
    fflush(fp);//冲洗内存流，然后文件位置为12，null(结束符\0)字节被加到字符串结尾处
    printf("after fflush: %s\n", buf);//前面写的数据已经写入内存流中了
    printf("len of string in buf = %ld\n", (long)strlen(buf));//输出起始位置到第一个结束符处的长度

    memset(buf, 'b', BSZ-2);
    buf[BSZ-2] = '\0';
    buf[BSZ-1] = 'X';//将buf的前46个字符置为'b',buf[46]置为结束符'\0'
    fprintf(fp, "hello, world");//向内存流的缓冲区中写入数据
    fseek(fp, 0, SEEK_SET);//先冲洗流，null(结束符\0)字节被加到字符串结尾处，再将内存流定位到起始位置
    printf("after  fseek: %s\n", buf);
    printf("len of string in buf = %ld\n", (long)strlen(buf));

    memset(buf, 'c', BSZ-2);
    buf[BSZ-2] = '\0';
    buf[BSZ-1] = 'X';//将buf的前46个字符置为'b',buf[46]置为结束符'\0'
    fprintf(fp, "hello, world");//向内存流的缓冲区中写入数据
    fclose(fp);//先冲洗流（这里由于写入的数据长度小于内存流实际长度，故未附加null），再关闭内存流
    printf("after fclose: %s\n", buf);
    printf("len of string in buf = %ld\n", (long)strlen(buf));

    return(0);
}
