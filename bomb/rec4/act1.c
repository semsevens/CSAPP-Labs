#include <stdio.h>

int main(int argc, char** argv)
{    
    int ret = printf("%s\n", argv[argc-1]);
    argv[0] = '\0'; // NOOP to force gcc to generate a callq instead of jmp
    return ret;
}