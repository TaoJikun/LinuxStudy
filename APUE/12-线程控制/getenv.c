/*
    getenv函数的非可重入版本
    因为所有调用getenv的线程返回的字符串都存放在同一个静态缓冲区中，
    所以这个版本不是可重入的，
    如果两个线程同时调用这个函数，就会看到不一致的结果。
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define MAXSTRINGSZ 4096

static char envbuf[MAXSTRINGSZ];

extern char ** environ;

char* getenv(const char* name)
{
    int i, len;

    len = strlen(name);
    for(i=0;environ[i]!=NULL;++i)
    {
        if((strncmp(name,environ[i],len) == 0) && 
            environ[i][len] == "=")
        {
            strncpy(envbuf, &environ[i][len+1], MAXSTRINGSZ-1);
            return envbuf;
        }
    }

    return NULL;
}
