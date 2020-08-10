#include <stdio.h>
#include <stdlib.h>

void compute(int inArg)
{
    int st;
    inArg *= 3;
    st = inArg & 0xf;
    do {
        switch(st)
        {
            case 0: printf("Finish\n"); inArg = -1; break;
            case 1: inArg *= 2; break;
            case 2: inArg >>= 2; break;
            case 3: inArg &= 1; break;
            case 4: inArg--; break;
            default:  break;
        }
        st = inArg & 0xf;
    } while (inArg >= 0);
}

int main(int argc, char** argv)
{
    int inArg;
    if (argc == 1) {fprintf(stderr, "Please rerun with a positive number argument\n"); return 1;}
    
    inArg = atoi(argv[1]);
    if (inArg < 0) {fprintf(stderr, "Argument was not a positive integer\n"); return 1;}
    compute(inArg);
    
    return 0;
}