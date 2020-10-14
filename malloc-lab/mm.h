/*
 * only check compile warning for mm*.c files, and report any warning as error
 * 
 * the original Makefile gcc flag is `-Wall -Wextra -Werror`
 * ref: http://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

#include <stdio.h>

#ifdef DRIVER

/* declare functions for driver tests */
extern void *mm_malloc(size_t size);
extern void mm_free(void *ptr);
extern void *mm_realloc(void *ptr, size_t size);
extern void *mm_calloc(size_t nmemb, size_t size);

#else

/* declare functions for interpositioning */
extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *realloc(void *ptr, size_t size);
extern void *calloc(size_t nmemb, size_t size);

#endif

extern int mm_init(void);

/* This is largely for debugging.  You can do what you want with the
   verbose flag; we don't care. */
extern void mm_checkheap(int verbose);

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define SIZE_PTR(p) ((size_t *)(((char *)(p)) - SIZE_T_SIZE))