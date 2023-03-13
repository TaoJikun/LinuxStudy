#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    __off_t curpos;
    if((curpos = lseek(STDIN_FILENO,0,SEEK_END)) == -1){
        printf("lseek error!\n");
    }
    else{
        printf("current position: %ld \n",(long int)curpos);
        printf("lseek OK!\n");
    }

    exit(0);
}