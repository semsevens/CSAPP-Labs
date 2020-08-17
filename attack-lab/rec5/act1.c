#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

void clobber(char*, int);

void printHi()
{
    printf("Hi!\n");
}

char* buf;

int main(int argc, char** argv)
{
    char* x = alloca(8);
    buf = malloc(16);
    *(long*) buf = (long)&printHi;
    *(long*) (buf + 8) = 0x0000000000400560;
    clobber(buf, 16);
    clobber(x, 8);
    return 0;
}