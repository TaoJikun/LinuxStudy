#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 4096

int main(void)
{
    char name[L_tmpnam], line[MAXLINE];
    FILE *fp;

    printf("%s \n", tmpnam(NULL));//获取静态区临时路径名的名称

    tmpnam(name);
    printf("%s \n",name);

    if((fp = tmpfile()) == NULL)//创建临时文件，并返回文件指针
    {
        printf("tmpfile() error!\n");
        exit(1);
    }

    fputs("one line of output!\n", fp);
    rewind(fp);//倒退到流的开头，如果没有这一步，fputs结束后文件指针指向文件的尾端，后面的fgets会失败

    if(fgets(line, sizeof(line),fp) == NULL)
    {
        printf("fgets() error!\n");
        exit(1);
    }

    fputs(line, stdout);

    exit(0);

}