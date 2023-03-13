#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

int func1(int i, int j, int k, int l);
void func2();

static jmp_buf jumpbuffer;
static int globval;

int main(void)
{
    int autoval;
    register int regival;
    volatile int volaval;
    static int statval;

    globval = 1; autoval = 2; regival = 3; volaval = 4; statval = 5;

    if (setjmp(jumpbuffer) != 0) {
        printf("after longjmp:\n");
        printf("globval = %d, autoval = %d, regival = %d,"
            " volaval = %d, statval = %d\n",
            globval, autoval, regival, volaval, statval);
        exit(0);
    }


    globval = 95; autoval = 96; regival = 97; volaval = 98;
    statval = 99;
    
    func1(autoval, regival, volaval, statval);	/* never returns */
    exit(0);
}


int func1(int i, int j, int k, int l)
{
    printf("in func1():\n");
    printf("globval = %d, autoval = %d, regival = %d,"
        " volaval = %d, statval = %d\n", globval, i, j, k, l);
    func2();
}


void func2()
{
    longjmp(jumpbuffer,1);
}