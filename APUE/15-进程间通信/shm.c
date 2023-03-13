#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>

#define ARRAY_SIZE 40000
#define MALLOC_SIZE 100000
#define SHM_SIZE 100000
#define SHM_MODE 0600

char array[ARRAY_SIZE]; //未初始化的全局数据，位于bss段

int main(void)
{
    int shmid;
    char *ptr, *shmptr;

    printf("array[] from %p to %p\n", (void *)&array[0], (void *)&array[ARRAY_SIZE]);

    printf("stack around %p\n", (void *)&shmid);

    if((ptr = malloc(MALLOC_SIZE)) == NULL)
    {
        printf("malloc() error!\n");
        exit(1);
    }
    printf("malloced from %p to %p\n", (void *)ptr, (void *)ptr+MALLOC_SIZE);
    
    if((shmid = shmget(IPC_PRIVATE, SHM_SIZE, SHM_MODE)) < 0)
    {
        printf("shmget() error!\n");
        exit(1);
    }

    if((shmptr = shmat(shmid, 0, 0)) == (void*)-1)
    {
        printf("shmat() error!\n");
        exit(1);
    }

    printf("shared memory attached from %p to %p\n", (void *)shmptr, (void *)shmptr+SHM_SIZE);

    if(shmctl(shmid, IPC_RMID, 0) < 0)
    {
        printf("shmctl() error!\n");
        exit(1);
    }

    exit(0);
}