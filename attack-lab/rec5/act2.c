#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <sys/mman.h>
#include <stdint.h>

void clobber(char*, int);
const char hiStr[] = "Hi\n";

int main(int argc, char** argv)
{
    char* x = alloca(32);
    unsigned char* m = malloc(128);
    
    puts("Activity 2!");
    if (m == NULL)
    {
        fprintf(stderr, "Allocation failure\n");
        return -1;
    }
    if (mprotect((void*)(((uint64_t)x) & (~0xfff)), 4096, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
    {
        perror("MPROTECT");
        free(m);
        return -1;
    }
    *(uint64_t*) m = (uint64_t)(x);
    m[8] = 0xbf;
    *(uint32_t*) (m + 9) = (unsigned int)(uint64_t) hiStr;
    *(uint32_t*) (m + 13) = 0x4022e0be;
    *(uint32_t*) (m + 18) = 0xd6ff;
    *(uint32_t*) (m + 20) = 0x4011a0be;
    *(uint32_t*) (m + 25) = 0xd6ff;
    clobber(m, 32);
    
    return 0;
}