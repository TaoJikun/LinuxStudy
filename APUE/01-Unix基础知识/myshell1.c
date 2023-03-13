#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAXLINE 4096

int main(int argc, char *argv[])
{
	char	buf[MAXLINE];
	pid_t	pid;
	int		status;

	printf("%% "); //打印print("%%")两个%%才能达到输出"%"的目的
	
    while (fgets(buf, MAXLINE, stdin) != NULL) 
    {
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; //将输入的指令字符串的最后一位的换行符改为0(NULL)，因为execpl函数需要求参数以null结尾

		if ((pid = fork()) < 0) {
			printf("fork error!\n");
            exit(1);
		} 
        else if (pid == 0) {		/* child */
			execlp(buf, buf, (char *)0); //exce协议族函数
			
            printf("couldn't execute: %s \n", buf);//重新加载输入的指令函数失败，直接终止子进程
			exit(127);
		}

		/* parent */
		if ((pid = waitpid(pid, &status, 0)) < 0)
        {
            printf("waitpid error!\n");
            exit(1);
        }
		printf("%% ");
	}
	exit(0);
}
