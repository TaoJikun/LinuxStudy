#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

//使用管道在父子进程之间进行通信，以解决竞争问题
static int pfd1[2], pfd2[2];

void charactatime(char *str);

void TELL_WAIT(void);
void TELL_PARENT(pid_t pid);
void WAIT_PARENT(void);
void TELL_CHILD(pid_t pid);
void WAIT_CHILD(void);


int main()
{
    __pid_t pid;

    TELL_WAIT();

    if ((pid = fork()) < 0)
    {
        printf("fork() error!\n");
    }
    else if(pid == 0)
    {
        WAIT_PARENT();
        charactatime("output from child!\n");
    }
    else
    {
        charactatime("output from parent!\n");
        TELL_CHILD(pid);
    }
    exit(0);
}


void charactatime(char *str)
{
    char *ptr;
    int c;

    setbuf(stdout,NULL);//将标准输出设置为无缓冲的，使得每一次输出都要进行I/O
    for(ptr = str;*ptr!=0;++ptr)
    {
        c=*ptr;
        putc(c,stdout);
    }
}



void TELL_WAIT(void)
{
    //创建管道
	if (pipe(pfd1) < 0 || pipe(pfd2) < 0)
		printf("pipe error\n");
}

void TELL_PARENT(pid_t pid)
{
    //向管道2中写入一个字符，通知父进程运行
	if (write(pfd2[1], "c", 1) != 1)
		printf("write error\n");
}

void WAIT_PARENT(void)
{
    //企图从管道1中读入字符，当读到了说明可以运行子进程了
	char c;

	if (read(pfd1[0], &c, 1) != 1)
		printf("read error\n");

	if (c != 'p')
		printf("WAIT_PARENT: incorrect data\n");
}

void TELL_CHILD(pid_t pid)
{
    //向管道1中写入一个字符，通知子进程运行
	if (write(pfd1[1], "p", 1) != 1)
		printf("write error\n");
}

void WAIT_CHILD(void)
{
    //企图从管道2中读入字符，当读到了说明可以运行子进程了
	char c;

	if (read(pfd2[0], &c, 1) != 1)
        printf("read error\n");

	if (c != 'c')
        printf("WAIT_CHILD: incorrect data\n");
}