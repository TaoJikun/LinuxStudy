#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 4096

int EchoSingleChar(void);

int EchoWholeRow(void);


int main(void)
{
    EchoWholeRow();
}


int EchoSingleChar(void)
{
    int c;
    
    while((c=getc(stdin)) != EOF)
    {
        if(putc(c,stdout) == EOF)
        {
            fputs("output error!\n",stdout);
            exit(1);
        }
    }

    if(feof(stdin)){
        fputs("end of file!\n",stdout);
        exit(0);
    }
    else if(ferror(stdin)){
        fputs("input error!\n",stdout);
        exit(1);
    }
}


int EchoWholeRow(void)
{
    char buf[MAXLINE];

    while ((fgets(buf,MAXLINE,stdin)) != NULL)
    {
        if (fputs(buf,stdout) == EOF)
        {
            fputs("output error!\n",stdout);
            exit(1);
        }
    }

    if(feof(stdin)){
        fputs("end of file!\n",stdout);
        exit(0);
    }
    else if(ferror(stdin)){
        fputs("input error!\n",stdout);
        exit(1);
    }
}