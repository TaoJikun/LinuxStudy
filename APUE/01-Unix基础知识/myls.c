#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char* argv[])
{
    DIR *dp;//目录流对象的数据类型
    struct dirent *dirp;

    if (argc != 2)
    {
        printf("usage: ls direction_name!\n");
        exit(1);
    }

    if((dp = opendir(argv[1])) == NULL)//打开输入所对应的路径，返回一个DIR指针
    {
        printf("can't open %s \n",argv[1]);
        exit(1);
    }
    
    while((dirp = readdir(dp)) != NULL)
        printf("%s \n",dirp->d_name);

    closedir(dp);
    exit(0);
    
}