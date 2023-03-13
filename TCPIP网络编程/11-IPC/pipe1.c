#include <stdio.h>
#include <unistd.h>

#define BUF_SIZE 30


int main(int argc, char *argv[])
{
    int fds[2];
    char str[] = "Who Are You?";
    char buf[BUF_SIZE];
    __pid_t pid;

    pipe(fds);
    pid = fork();
    if(pid == 0){
        write(fds[1],str,sizeof(str));
        printf("Child Process fds: %d \n",fds[1]);
    }
    else{
        read(fds[0],buf,BUF_SIZE);
        printf("Parent Process fds: %d \n",fds[0]);
        puts(buf);
    }

    return 0;
}
