/*
 * mm-explicit.c
 * 
 * explicit free list
 * - no circle list
 * - first fit
 * - LIFO (free to the header)
 */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Basic constants */
typedef uint32_t word_t;
static const size_t chunksize = (1 << 12);     // requires (chunksize % 8 == 0)
static const size_t wsize = sizeof(word_t);    // word and header size (bytes)
static const size_t dsize = 2 * wsize;         // double word size (bytes)
static const size_t tsize = 3 * wsize;         // triple word size (bytes)
static const size_t ptr_size = sizeof(char *); // pointer size (bytes)
/*
 * Minimum block size, (header + footer + 2 * pointer + struct inner padding)
 */
static const size_t min_block_size = 2 * wsize + 2 * ptr_size + 2 * wsize;

static const word_t alloc_mask = 0x1;
static const word_t size_mask = ~(word_t)0x7;

typedef struct block {
    /* Header contains size + allocation flag */
    word_t header;
    /* dummy word to make payload aligned at 8-multiple */
    word_t _dummy;
    /*
     * We don't know how big the payload will be.  Declaring it as an
     * array of size 0 allows computing its starting address using
     * pointer notation.
     */
    char payload[0];
    /*
     * explicit free list pointer
     * pred: predecessor, point to the previous free block in explicit free list
     * succ: successor, point to the next free block in explicit free list
     * 
     * !!NOTICE!!: due to struct alignment requirement, pred and succ will align 
     * at 8 multiple address, which cause a 4-byte hole after header
     */
    struct block *pred;
    struct block *succ;
    /*
     * We can't declare the footer as part of the struct, since its starting
     * position is unknown
     */
    // word_t footer; // CANNOT uncomment this line
} block_t;

static const size_t block_with_footer_size = sizeof(block_t) + dsize;

/* Global variables */
/* Pointer to first block */
static block_t *heap_listp = NULL;
/* Pointer to explicit free list */
static block_t *free_listp = NULL;

/* Function prototypes for internal helper routines */
static block_t *extend_heap(size_t size);
static void place(block_t *block, size_t asize);
static block_t *find_fit(size_t asize);
static block_t *coalesce(block_t *block);

static size_t max(size_t x, size_t y);
static size_t min(size_t x, size_t y);
static size_t round_up(size_t size, size_t n);
static word_t pack(size_t size, bool alloc);

static size_t extract_size(word_t header);
static size_t get_size(block_t *block);
static size_t get_payload_size(block_t *block);

static bool extract_alloc(word_t header);
static bool get_alloc(block_t *block);

static void write_header(block_t *block, size_t size, bool alloc);
static void write_footer(block_t *block, size_t size, bool alloc);

static block_t *payload_to_header(void *pp);
static void *header_to_payload(block_t *block);

static block_t *find_next(block_t *block);
static word_t *find_prev_footer(block_t *block);
static block_t *find_prev(block_t *block);

static void insert_free_block(block_t *bp);
static void remove_from_free_list(block_t *bp);

bool mm_checkheap(int lineno);

/*
 * mm_init
 * 
 * initialize the heap, it is run once when heap_start == NULL.
 */
int mm_init(void) {
    // Create the initial empty heap
    block_t *start = (block_t *)(mem_sbrk(2 * block_with_footer_size));

    if (start == (void *)-1) {
        return -1;
    }

    write_header(start, block_with_footer_size, true); // Prologue header
    write_footer(start, block_with_footer_size, true); // Prologue footer
    block_t *epilogue = find_next(&(start[0]));
    start[0].succ = epilogue; // Prologue successor

    write_header(epilogue, 0, true); // Epilogue header
    epilogue->pred = &(start[0]);    // Epilogue predecessor
    epilogue->succ = NULL;           // Epilogue successor

    // Heap starts with prologue footer
    heap_listp = (block_t *)&(start[0]);

    // Explicit free list point to the very start of virtual address space
    free_listp = (block_t *)&(start[0]);

    // Extend the empty heap with a free block of chunksize bytes
    if (extend_heap(chunksize) == NULL) {
        return -1;
    }
    return 0;
}

/*
 * malloc
 */
void *malloc(size_t size) {
    dbg_requires(mm_checkheap(__LINE__));

    size_t asize;      // Adjusted block size
    size_t extendsize; // Amount to extend heap if no fit is found
    block_t *block;
    void *bp = NULL;

    // Ignore spurious request
    if (size == 0) {
        dbg_ensures(mm_checkheap(__LINE__));
        return bp;
    }

    // Initialize heap if it isn't initialized
    if (heap_listp == NULL) {
        mm_init();
    }

    // Adjust block size to include overhead and to meet alignment requirements
    asize = min(round_up(size, dsize) + dsize, min_block_size);

    // Search the free list for a fit
    block = find_fit(asize);

    // If no fit is found, request more memory, and then and place the block
    if (block == NULL) {
        extendsize = max(asize, chunksize);
        block = extend_heap(extendsize);
        // extend_heap returns an error
        if (block == NULL) {
            return bp;
        }
    }

    place(block, asize);
    bp = header_to_payload(block);

    dbg_printf("Malloc size %zd on address %p.\n", size, bp);
    dbg_ensures(mm_checkheap(__LINE__));
    return bp;
}

/*
 * free
 */
void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    dbg_printf("free %p\n", ptr);

    block_t *block = payload_to_header(ptr);
    size_t size = get_size(block);

    write_header(block, size, false);
    write_footer(block, size, false);

    coalesce(block);
}

/*
 * realloc
 */
void *realloc(void *oldptr, size_t size) {
    oldptr = oldptr;
    size = size;
    return NULL;
}

/*
 * calloc
 */
void *calloc(size_t nmemb, size_t size) {
    nmemb = nmemb;
    size = size;
    return NULL;
}

/******** The remaining content below are helper and debug routines ********/

/*
 * extend_heap: Extends the heap with the requested number of bytes, and
 *              recreates epilogue header. Returns a pointer to the result of
 *              coalescing the newly-created block with previous free block, if
 *              applicable, or NULL in failure.
 */
static block_t *extend_heap(size_t size) {
    void *bp;

    // Allocate an even number of words to maintain alignment
    size = round_up(size, dsize);
    if ((bp = mem_sbrk(size)) == (void *)-1) {
        return NULL;
    }

    // Initialize free block header/footer
    block_t *block = payload_to_header(bp);
    write_header(block, size, false);
    write_footer(block, size, false);
    // Create new epilogue header
    block_t *block_next = find_next(block);
    write_header(block_next, 0, true);

    // Coalesce in case the previous block was free
    return coalesce(block);
}

/* Coalesce: Coalesces current block with previous and next blocks if either
 *           or both are unallocated; otherwise the block is not modified.
 *           Returns pointer to the coalesced block. After coalescing, the
 *           immediate contiguous previous and next blocks must be allocated.
 */
static block_t *coalesce(block_t *block) {
    block_t *block_next = find_next(block);
    block_t *block_prev = find_prev(block);

    bool prev_alloc = get_alloc(block_prev);
    bool next_alloc = get_alloc(block_next);
    size_t size = get_size(block);

    // Case 1
    if (prev_alloc && next_alloc) {

    }
    // Case 2
    else if (prev_alloc && !next_alloc) {
        size += get_size(block_next);
        write_header(block, size, false);
        write_footer(block, size, false);
    }
    // Case 3
    else if (!prev_alloc && next_alloc) {
        size += get_size(block_prev);
        write_header(block_prev, size, false);
        write_footer(block_prev, size, false);
        block = block_prev;
    }
    // Case 4
    else {
        size += get_size(block_next) + get_size(block_prev);
        write_header(block_prev, size, false);
        write_footer(block_prev, size, false);
        block = block_prev;
    }

    insert_free_block(block);
    return block;
}

/*
 * place: Places block with size of asize at the start of bp. If the remaining
 *        size is at least the minimum block size, then split the block to the
 *        the allocated block and the remaining block as free.
 */
static void place(block_t *block, size_t asize) {
    size_t csize = get_size(block);

    remove_from_free_list(block);

    if ((csize - asize) >= min_block_size) {
        write_header(block, asize, true);
        write_footer(block, asize, true);

        block_t *block_next = find_next(block);
        write_header(block_next, csize - asize, false);
        write_footer(block_next, csize - asize, false);
        insert_free_block(block_next);
    } else {
        write_header(block, csize, true);
        write_footer(block, csize, true);
    }
}

/*
 * find_fit: Looks for a free block with at least asize bytes with
 *           first-fit policy. Returns NULL if none is found.
 */
static block_t *find_fit(size_t asize) {
    block_t *block;

    for (block = free_listp->succ; block->succ != NULL; block = block->succ) {
        if (!get_alloc(block) && (asize <= get_size(block))) {
            return block;
        }
    }

    return NULL;
}

/*
 * max: returns x if x > y, and y otherwise.
 */
static size_t max(size_t x, size_t y) {
    return (x > y) ? x : y;
}

/*
 * min: returns x if x < y, and y otherwise.
 */
static size_t min(size_t x, size_t y) {
    return (x < y) ? x : y;
}

/*
 * round_up: Rounds size up to next multiple of n
 */
static size_t round_up(size_t size, size_t n) {
    return (n * ((size + (n - 1)) / n));
}

/*
 * pack: returns a header reflecting a specified size and its alloc status.
 *       If the block is allocated, the lowest bit is set to 1, and 0 otherwise.
 */
static word_t pack(size_t size, bool alloc) {
    return alloc ? (size | 1) : size;
}

/*
 * extract_size: returns the size of a given header value based on the header
 *               specification above.
 */
static size_t extract_size(word_t word) {
    return (word & size_mask);
}

/*
 * get_size: returns the size of a given block by clearing the lowest 3 bits
 *           (as the heap is 8-byte aligned).
 */
static size_t get_size(block_t *block) {
    return extract_size(block->header);
}

/*
 * get_payload_size: returns the payload size of a given block, equal to
 *                   the entire block size minus the header and footer sizes.
 */
static size_t get_payload_size(block_t *block) {
    size_t asize = get_size(block);
    return asize - tsize;
}

/*
 * extract_alloc: returns the allocation status of a given header value based
 *                on the header specification above.
 */
static bool extract_alloc(word_t word) {
    return (bool)(word & alloc_mask);
}

/*
 * get_alloc: returns true when the block is allocated based on the
 *            block header's lowest bit, and false otherwise.
 */
static bool get_alloc(block_t *block) {
    return extract_alloc(block->header);
}

/*
 * write_header: given a block and its size and allocation status,
 *               writes an appropriate value to the block header.
 */
static void write_header(block_t *block, size_t size, bool alloc) {
    block->header = pack(size, alloc);
}

/*
 * write_footer: given a block and its size and allocation status,
 *               writes an appropriate value to the block footer by first
 *               computing the position of the footer.
 */
static void write_footer(block_t *block, size_t size, bool alloc) {
    word_t *footerp = (word_t *)((block->payload) + get_payload_size(block));
    *footerp = pack(size, alloc);
}

/*
 * find_next: returns the next consecutive block on the heap by adding the
 *            size of the block.
 */
static block_t *find_next(block_t *block) {
    dbg_requires(block != NULL);
    block_t *block_next = (block_t *)(((char *)block) + get_size(block));

    dbg_ensures(block_next != NULL);
    return block_next;
}

/*
 * find_prev_footer: returns the footer of the previous block.
 */
static word_t *find_prev_footer(block_t *block) {
    // Compute previous footer position as one word before the header
    return (&(block->header)) - 1;
}

/*
 * find_prev: returns the previous block position by checking the previous
 *            block's footer and calculating the start of the previous block
 *            based on its size.
 */
static block_t *find_prev(block_t *block) {
    word_t *footerp = find_prev_footer(block);
    size_t size = extract_size(*footerp);
    return (block_t *)((char *)block - size);
}

/*
 * payload_to_header: given a payload pointer, returns a pointer to the
 *                    corresponding block.
 */
static block_t *payload_to_header(void *pp) {
    return (block_t *)(((char *)pp) - offsetof(block_t, payload));
}

/*
 * header_to_payload: given a block pointer, returns a pointer to the
 *                    corresponding payload.
 */
static void *header_to_payload(block_t *block) {
    return (void *)(block->payload);
}

/*
 * Insert new alloced block to free list, LIFO style
 */
static void insert_free_block(block_t *bp) {
    bp->pred = free_listp;
    bp->succ = free_listp->succ;
    bp->pred->succ = bp;
    bp->succ->pred = bp;
}

/*
 * Remove this block from free list
 */
static void remove_from_free_list(block_t *bp) {
    bp->pred->succ = bp->succ;
    bp->succ->pred = bp->pred;
}

/*
 * mm_checkheap
 */
bool mm_checkheap(int lineno) {
    return (bool)lineno;
}
