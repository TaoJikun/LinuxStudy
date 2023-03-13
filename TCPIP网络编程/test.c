#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main()
{
	int fd[2] = { 0 };
	if (pipe(fd) < 0){ //使用pipe创建匿名管道
		perror("pipe");
		return 1;
	}
	pid_t id = fork(); //使用fork创建子进程
	if (id == 0){
		//child 
		close(fd[0]); //子进程关闭读端
		//sleep(2);
		char c = 'a';
		int count = 0;
		//子进程一直进行写入，一次写入一个字节
		while (1){
			write(fd[1], &c, 1);
			count++;
			printf("%d\n", count); //打印当前写入的字节数
		}
		close(fd[1]);
		exit(0);
	}
	//father
	close(fd[1]); //父进程关闭写端

	//父进程不进行读取

    close(fd[0]);

	waitpid(id, NULL, 0);

	return 0;
}