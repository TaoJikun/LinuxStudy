#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define MAXLINE 4096
#define TOK_ADD 5

void do_line(char *ptr);
void cmd_add(void);
int get_token(void);

char *tock_ptr;

jmp_buf jumpbuf;

int main(void)
{
    char line[MAXLINE];

    if (setjmp(jumpbuf) != 0)
    {
        printf("error!\n");
    }
    

    while (fgets(line,MAXLINE,stdin) != NULL)
    {
        do_line(line);
    }

    exit(0);
    
}

void do_line(char *ptr)
{
    int cmd;

    tock_ptr = ptr;
    while ((cmd = get_token()) > 0)
    {
        switch (cmd)
        {
        case TOK_ADD:
            cmd_add();
            break;
        
        default:
            break;
        }
    }

    longjmp(jumpbuf,1);
    
}


void cmd_add(void)
{
    int token;
    token = get_token();

    longjmp(jumpbuf,2);
}


int get_token(void)
{
    return 5;
}