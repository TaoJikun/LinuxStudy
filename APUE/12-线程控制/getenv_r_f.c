#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>

#define MAXSTRINGSZ 4096

static pthread_key_t key;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;
pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;
extern char** environ;

static void thread_init(void)
{
    pthread_key_create(key, free);
}

char* getenv_r_f(const char* name)
{
    int i, len;
    char *envbuf;

    pthread_once(init_done,thread_init);

    pthread_mutex_lock(&env_mutex);
    envbuf = (char*)pthread_getspecific(key);//获取线程特定数据值
    if (envbuf == NULL)
    {
        //没有值与键值key相关联
        envbuf = malloc(MAXSTRINGSZ);
        if(envbuf == NULL)
        {
            pthread_mutex_unlock(&env_mutex);
            return(NULL);
        }
        pthread_setspecific(key, envbuf);
    }
    
    len = strlen(name);
    for(i=0;environ[i]!=NULL;++i)
    {
        if((strncmp(name, environ[i], len) == 0) &&
            environ[i][len] == '=')
        {
            strncpy(envbuf, &environ[i][len+1], MAXSTRINGSZ-1);
            pthread_mutex_unlock(&env_mutex);
            return(envbuf);
        }
    }
    
    pthread_mutex_unlock(&env_mutex);
    return(NULL);
}