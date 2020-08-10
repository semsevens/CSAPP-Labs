#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int compare(int a, int b);

int main(int argc, char** argv)
{
    int a,b;
    if (argc!=3) {
       printf("usage: %s num1 num2\n",argv[0]);
       exit(-1);
    }

    a=atoi(argv[1]);
    b=atoi(argv[2]);
    if (compare(a,b)==1) {
       printf("good args!\n");
    }
    else {
       printf("bad numbers, try harder!\n");
    }
    return 0;
}
