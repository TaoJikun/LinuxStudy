#include <stdio.h>
#include <unistd.h>

#define BUF_SIZE 30


int main(int argc, char *argv[])
{
    int fds[2];
    char str1[] = "Who Are You?";
    char str2[] = "Thank you for your message.";
    char buf[BUF_SIZE];
    __pid_t pid;

    pipe(fds);
    pid = fork();
    if(pid == 0){
        write(fds[1],str1,sizeof(str1));
        sleep(2);
        read(fds[0],buf,BUF_SIZE);
        printf("Child Process Output: %s \n",buf);
    }
    else{
        read(fds[0],buf,BUF_SIZE);
        printf("Parent Process Output: %s \n",buf);
        write(fds[1],str2,sizeof(str2));
        sleep(3);//防止父进程先终止时弹出命令提示符
    }

    return 0;
}
