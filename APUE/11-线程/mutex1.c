/*
    互斥量保护数据结构
*/
#include <stdlib.h>
#include <pthread.h>

//在使用该对象前，线程需要对这个对象的引用计数加1，
//当对象使用完毕时，需要对引用计数减1。
//当最后一个引用被释放时，对象所占的内存空间就被释放。
//在对引用计数加1、减1以及检查引用计数是否为0，这些操作之前需要锁住互斥量。
struct foo{
    int f_count;
    pthread_mutex_t f_lock;
    int f_id;
};

struct foo* foo_alloc(int id)
{
    struct foo *fp;

    if((fp=malloc(sizeof(struct foo))) != NULL)
    {
        fp->f_count=1;
        fp->f_id = id;
        if(pthread_mutex_init(&fp->f_lock,NULL) != 0)
        {
            free(fp);
            return NULL;
        }
    }

    return fp;
}

void foo_hold(struct foo *fp)
{
    pthread_mutex_lock(&fp->f_lock);
    fp->f_count++;
    pthread_mutex_unlock(&fp->f_lock);
}

void foo_rele(struct foo *fp)
{
    pthread_mutex_lock(&fp->f_lock);
    if(--fp->f_count == 0)
    {
        pthread_mutex_unlock(&fp->f_lock);
        pthread_mutex_destroy(&fp->f_lock);
        free(fp);
    }
    else
    {
        pthread_mutex_unlock(&fp->f_lock);
    }
}