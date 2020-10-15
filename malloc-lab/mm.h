/*
 * header for mm.c
 */

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

/*
 * If DEBUG is defined, enable printing on dbg_printf and contracts.
 * Debugging macros, with names beginning "dbg_" are allowed.
 * You may not define any other macros having arguments.
 */

#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#else
/* When debugging is disnabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#endif
