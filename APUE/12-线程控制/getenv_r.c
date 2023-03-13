/*
    getenv的可重入版本，叫做genenv_r
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

extern char** environ;
pthread_mutex_t env_mutex;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;

static void thread_init(void)
{
    pthread_mutexattr_t attr;

    pthread_attr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&env_mutex,&attr);
    pthread_attr_destroy(&attr);
}

int getenv_r(const char *name, char *buf, int buflen)
{
    int i,len,olen;
    pthread_once(init_done,thread_init);//保证thread_init函数在本进程执行序列中仅执行一次。
    len = strlen(name);
    pthread_mutex_lock(&env_mutex);//加锁是为了在搜索请求的字符串时保护环境不被修改，这样才能做到线程安全
    for(i=0;environ[i]!=NULL;++i)
    {
        if((strncmp(name, environ[i], len) == 0) &&
            environ[i][len] == '=')
        {
            olen = strlen(environ[i][len+1]);
            if(olen >= buflen)
            {
                pthread_mutex_unlock(&env_mutex);
                return (ENOSPC);
            }
            strcpy(buf, &environ[i][len+1]);
            pthread_mutex_unlock(&env_mutex);
            return (0);
        }
    }
    pthread_mutex_unlock(&env_mutex);
    return (ENOSPC);
}