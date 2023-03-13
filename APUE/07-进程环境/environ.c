#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main(int argc, char *argv[])
{
    for (int i = 0; environ[i]!=NULL; i++)
    {
        printf("environ[%d]: %s \n",i,environ[i]);
        printf("environ[%d] position: %p \n", i, environ+i);
        printf("environ[%d]  char position: %p \n", i, *(environ+i));
    }

    printf("sizeof ptr: %ld \n",sizeof(environ));

    exit(0);
}