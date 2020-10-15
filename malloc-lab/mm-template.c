/*
 * mm-template.c
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/*
 * mm_init
 */
int mm_init(void) {
    return 0;
}

/*
 * malloc
 */
void *malloc(size_t size) {
    return NULL;
}

/*
 * free
 */
void free(void *ptr) {
}

/*
 * realloc
 */
void *realloc(void *oldptr, size_t size) {
    return NULL;
}

/*
 * calloc
 */
void *calloc(size_t nmemb, size_t size) {
    return NULL;
}

/*
 * mm_checkheap
 */
void mm_checkheap(int verbose) {
}
