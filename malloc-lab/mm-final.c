/*
 * mm-final.c
 * 
 * based on mm-segregated.c, apply as many optimization as possible
 * 
 * segregated free list
 * - min_block_size: 16
 * - 16 size class, (0,16],(16,24],(24,32],(32,48],(48,64],(64, 128],...,(2^(i+1), 2^(i+2)],...,(2^16, +inf)
 * - circle explicit free list
 * - first fit
 * - LIFO (free to the header of size class)
 * 
 * optimizations:
 * 
 * # binary search for highest 1 bit in `get_highest_1bit_idx`
 *      mm-segregated version is implemented in bit by bit loop style, which takes O(n) time
 *      so optimize it to use binary search style, which takes O(log(n)) time
 *      
 *      this optimization can increase throughput
 * 
 * # reduce `pred`, `succ` size by replacing with `pred_offset`, `succ_offset` compared to the beginning of the heap
 *      this optimization is based on the fact, which quoted below from malloc lab writeup:
 *          > the size of the heap will never be greater than or equal to 2^32 bytes. 
 *          > This does not imply anything about the location of the heap, 
 *          > but there is a neat optimization that can be done using this information.
 *      
 *      the fact means that block address can be calculated from an offset and the heap start address,
 *      which means the `pred`, `succ` in `block_t` can be replaced by `pred_offset`, `succ_offset`.
 *      by replacing, we can reduce 8 bytes of `pred` to 4 bytes of `pred_offset`, same for `succ`
 * 
 *      notice that this optimization will also make minimal block size to 16, which will also change the size class
 * 
 *      this optimization can increase space utilization
 * 
 * # footerless, only need to maintain footer for free block
 *      this optimization is inspired from lecture: <Dynamic Memory Allocation: Basic>:
 *          > Boundary tag needed only for free blocks
 *          > When sizes are multiples of 4 or more, have 2+ spare bits
 *      
 *      this optimization can increase space utilization
 * 
 * # replace frequently used inline functions with macros
 *      controlled by compile flag `USE_MACRO`
 * 
 *      this optimization can increase throughput
 * 
 * # dynamically adjust `extend_heap` chunksize
 *      dynamically increase or decrease `chunksize` by factor of 2
 *      - for large extend request, increase `chunksize` by factor of 2
 *      - for small extend request, decrease `chunksize` by factor of 2
 * 
 *      this optimization can increase space utilization
 * 
 * # use gcc builtin function to calculate highest 1 bit
 *      ref: https://stackoverflow.com/questions/671815/what-is-the-fastest-most-efficient-way-to-find-the-highest-set-bit-msb-in-an-i
 * 
 *      this optimization can increase throughput
 * 
 * # more fine granularity size class
 *      - separate classes for each small size
 *      - for larger size: one class for each size (2^i, 2^(i+1)]
 * 
 *      this optimization can increase space utilization
 * 
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
static const size_t max_chunksize = (1 << 8); // chunksize cap for dynamic extend heap
static const size_t min_chunksize = (1 << 4); // minimal chunksize for dynamic extend heap
static const size_t wsize = sizeof(word_t);   // word and header size (bytes)
static const size_t dsize = 2 * wsize;        // double word size (bytes)
static const size_t size_class_cnt = 16;      // how many size class in segregated list

static const word_t alloc_mask = 0x1;         // current block alloc bit at lowest bit
static const word_t prev_alloc_mask = 0x2;    // previous block alloc bit at second lowest bit
static const word_t size_mask = ~(word_t)0x7; // current block size at high 29 bits (because of size is 8-multiple)

typedef struct block {
    /* Header contains size + allocation flag */
    word_t header;

    /*
     * We don't know how big the payload will be.
     * Declaring it as an array of size 0 allows computing its starting address using
     * pointer notation.
     */
    char payload[0];

    /*
     * explicit free list pointer, represented by offset
     * pred_offset: address offset of predecessor, point to the previous free block in explicit free list
     * succ_offset: address offset of successor, point to the next free block in explicit free list
     */
    word_t pred_offset;
    word_t succ_offset;

    /*
     * We can't declare the footer as part of the struct, since its starting
     * position is unknown
     */
    // word_t footer; // CANNOT uncomment this line
} block_t;

/**
 * block size
 */
static const size_t block_size = sizeof(block_t);
/*
 * Minimum block size, (header + footer + 2 * pointer offset)
 */
static const size_t min_block_size = 2 * wsize + 2 * wsize;
/**
 * Prologue block size, header + footer
 */
static const size_t prologue_size = 2 * wsize;
/*
 * Epilogue block size, header
 */
static const size_t epilogue_size = wsize;

/* Global variables */
/* Pointer to first block */
static block_t *heap_listp = NULL;
/* Pointer to segregated free list, of `size_class_cnt` length */
static block_t *seglist = NULL;
/* Pointer to the beginning of heap */
static char *heap_start = NULL;
/**
 * chunksize
 * 
 * initially set to a small number, but dynamically increase by double size,
 * but no larger than max_chunksize
 * 
 * requires (chunksize % 8 == 0)
 */
static size_t chunksize = min_chunksize;

/* Function prototypes for internal helper routines */
static block_t *extend_heap(size_t size);
static void place(block_t *block, size_t asize);
static block_t *find_fit(size_t asize);
static block_t *coalesce(block_t *block);

#ifdef USE_MACRO
/**
 * these functions are most frequently used, so replace them with macros
 */
#define max(x, y) ((x > y) ? x : y)
#define min(x, y) ((x < y) ? x : y)
#define round_up(size, n) (n * ((size + (n - 1)) / n))
#define get_asize(size) (round_up(size + dsize, dsize))
#define pack(size, alloc, prev_alloc) (size | alloc | (prev_alloc ? 2 : 0))

#define extract_size(word) ((size_t)(word & size_mask))
#define get_size(block) (extract_size(block->header))
#define get_payload_size(block) (get_size(block) - dsize)

#define extract_alloc(word) ((bool)(word & alloc_mask))
#define get_alloc(block) (extract_alloc(block->header))

#define extract_prev_alloc(word) ((bool)(word & prev_alloc_mask))
#define get_prev_alloc(block) (extract_prev_alloc(block->header))

#define write_header(block, size, alloc, prev_alloc) (block->header = pack(size, alloc, prev_alloc))
#define write_footer(block, size, alloc, prev_alloc) (*(get_footer(block)) = pack(size, alloc, prev_alloc))
#define get_header(block) (&(block->header))
#define get_footer(block) ((word_t *)((block->payload) + get_payload_size(block)))

#define find_next(block) ((block_t *)(((char *)block) + get_size(block)))
#define find_prev_footer(block) ((&(block->header)) - 1)
#define find_prev(block) ((block_t *)((char *)block - extract_size(*(find_prev_footer(block)))))

#define payload_to_header(pp) ((block_t *)(((char *)pp) - offsetof(block_t, payload)))
#define header_to_payload(block) ((void *)(block->payload))

#define get_block_ptr_by_offset(offset) ((block_t *)(heap_start + offset))
#define get_offset_by_block_ptr(bp) ((word_t)(((char *)bp) - heap_start))

#define mark_prev_alloc_as_free(block) (block->header &= (~0x2))
#define mark_prev_alloc_as_alloced(block) (block->header |= 0x02)
#define mark_as_free(block) (block->header = (*(get_footer(block))) = (block->header & (~alloc_mask)))
#else

static size_t max(size_t x, size_t y);
static size_t min(size_t x, size_t y);
static size_t round_up(size_t size, size_t n);
static size_t get_asize(size_t size);
static word_t pack(size_t size, bool alloc, bool prev_alloc);

static size_t extract_size(word_t word);
static size_t get_size(block_t *block);
static size_t get_payload_size(block_t *block);

static bool extract_alloc(word_t word);
static bool get_alloc(block_t *block);

static bool extract_prev_alloc(word_t word);
static bool get_prev_alloc(block_t *block);

static void write_header(block_t *block, size_t size, bool alloc, bool prev_alloc);
static void write_footer(block_t *block, size_t size, bool alloc, bool prev_alloc);
static word_t *get_header(block_t *block);
static word_t *get_footer(block_t *block);

static block_t *find_next(block_t *block);
static word_t *find_prev_footer(block_t *block);
static block_t *find_prev(block_t *block);

static block_t *payload_to_header(void *pp);
static void *header_to_payload(block_t *block);

static block_t *get_block_ptr_by_offset(word_t offset);
static word_t get_offset_by_block_ptr(block_t *bp);

static void mark_prev_alloc_as_free(block_t *block);
static void mark_prev_alloc_as_alloced(block_t *block);
static void mark_as_free(block_t *block);
#endif

static void insert_free_block(block_t *bp);
static void remove_from_free_list(block_t *bp);

static void split_block(block_t *bp, size_t asize);

static uint32_t get_seglist_idx(size_t size);
static uint32_t get_highest_1bit_idx(uint32_t num);

static void dbg_print_heap(bool skip);
#ifdef PRINT_HEAP
#define print_heap(...) dbg_print_heap(true)
#else
#define print_heap(...) dbg_print_heap(false)
#endif

#ifdef CHECK_HEAP
#define checkheap(...) mm_checkheap(__VA_ARGS__)
#else
#define checkheap(...) mm_checkheap(0)
#endif

/*
 * mm_init
 * initialize the heap, it is run once when heap_start == NULL.
 * 
 * prior to any extend_heap operation, this is the heap:
 * start               start+12              start+12*2  start+12*15               start+12*16             start+12*16+4     start+12*16+4+4   start+12*16+4+4+4 start+12*16+4+4+4+4
 *   | size class (0, 16] | size class (16, 32] | ..., .... | size class (2^18, +inf] | unused alignment word | prologue header | prologue footer | epilogue header |
 * 
 * each size class is of type `block_t`, but only `pred_offset`, `succ_offset` matters, because of circle list for each explicit list
 * 
 * prologue header/footer and epilogue header is mainly for coalesce simplicity
 */
int mm_init(void) {
    // Check `get_highest_1bit_idx` helper function correctness
    dbg_assert(get_highest_1bit_idx(1 << 0) == 1);
    dbg_assert(get_highest_1bit_idx(1 << 5) == 6);
    dbg_assert(get_highest_1bit_idx(1 << 9) == 10);
    dbg_assert(get_highest_1bit_idx((1 << 2) - 1) == 2);
    dbg_assert(get_highest_1bit_idx((1 << 10) - 1) == 10);

    // Check `get_seglist_idx` helper function correctness
    dbg_assert(get_seglist_idx(1) == 0);
    dbg_assert(get_seglist_idx(16) == 0);
    dbg_assert(get_seglist_idx(20) == 1);
    dbg_assert(get_seglist_idx(24) == 1);
    dbg_assert(get_seglist_idx(32) == 2);
    dbg_assert(get_seglist_idx(33) == 3);
    dbg_assert(get_seglist_idx(42) == 3);
    dbg_assert(get_seglist_idx(64) == 4);
    dbg_assert(get_seglist_idx(65) == 5);
    dbg_assert(get_seglist_idx(128) == 5);
    dbg_assert(get_seglist_idx(160) == 6);
    dbg_assert(get_seglist_idx(200) == 6);
    dbg_assert(get_seglist_idx(300) == 7);
    dbg_assert(get_seglist_idx(400) == 7);
    dbg_assert(get_seglist_idx(500) == 7);
    dbg_assert(get_seglist_idx(512) == 7);
    dbg_assert(get_seglist_idx(513) == 8);
    dbg_assert(get_seglist_idx(1 << 10) == 8);
    dbg_assert(get_seglist_idx((1 << 12) - 1) == 10);
    dbg_assert(get_seglist_idx(1 << 14) == 12);
    dbg_assert(get_seglist_idx((1 << 14) + 1) == 13);
    dbg_assert(get_seglist_idx((1 << 15) - 1) == 13);
    dbg_assert(get_seglist_idx(1 << 15) == 13);
    dbg_assert(get_seglist_idx((1 << 15) + 1) == 14);
    dbg_assert(get_seglist_idx((1 << 16) + 1) == 15);
    dbg_assert(get_seglist_idx((1 << 19) + 1) == 15);
    dbg_assert(get_seglist_idx(1 << 22) == 15);

    // Create the initial empty heap
    block_t *start = (block_t *)(mem_sbrk(size_class_cnt * block_size + wsize + prologue_size + epilogue_size));

    if (start == (void *)-1)
        return -1;

    // record the very beginning of heap
    heap_start = (char *)start;

    /**
     * Initialize the explicit free list of each size class bucket
     * pred_offset and succ_offset set to the belonging size class bucket address offset initially
     */
    for (size_t i = 0; i < size_class_cnt; i++) {
        start[i].pred_offset = get_offset_by_block_ptr(&(start[i]));
        start[i].succ_offset = get_offset_by_block_ptr(&(start[i]));
    }
    /**
     * set seglist to the very beginning of heap, 
     * which makes seglist an array of `block_t[size_class_cnt]`
     */
    seglist = start;

    /**
     * prologue starts after seglist + dummy alignment word
     * this alignment word is to make payload aligned at 8-multiple
     */
    block_t *prologue = (block_t *)((char *)&(start[size_class_cnt]) + wsize);
    write_header(prologue, prologue_size, true, true);

    // epilogue starts immediately after prologue
    block_t *epilogue = (block_t *)((char *)prologue + prologue_size);
    write_header(epilogue, 0, true, true);

    // heap starts with prologue
    heap_listp = prologue;

    if (extend_heap(chunksize) == NULL)
        return -1;

    return 0;
}

/*
 * malloc
 * 
 * quote from textbook:
 *      To allocate a block, we determine the size class of the request 
 *      and do a ﬁrst ﬁt search of the appropriate free list for a block that ﬁts. 
 *      If we ﬁnd one, then we (optionally) split it and insert the fragment in the appropriate free list. 
 *      If we cannot ﬁnd a block that ﬁts, then we search the free list for the next larger size class.
 *      We repeat until we ﬁnd a block that ﬁts. If none of the free lists yields a block that ﬁts, 
 *      then we request additional heap memory from the operating system, 
 *      allocate the block out of this new heap memory, 
 *      and place the remainder in the appropriate size class.
 */
void *malloc(size_t size) {
    assert(checkheap(__LINE__));

    size_t asize;      // Adjusted block size
    size_t extendsize; // Amount to extend heap if no fit is found
    block_t *block;
    void *bp = NULL;

    // Ignore spurious request
    if (size == 0) {
        assert(checkheap(__LINE__));
        return bp;
    }

    // Initialize heap if it isn't initialized
    if (heap_listp == NULL) {
        mm_init();
    }

    asize = get_asize(size);
    // Search the free list for a fit
    block = find_fit(asize);

    // If no fit is found, request more memory, and then and place the block
    if (block == NULL) {
        if (asize > chunksize) {
            extendsize = asize;
            // request larger chunk, so dynamically increase chunksize by factor of 2
            chunksize = min(max_chunksize, chunksize << 1);
        } else {
            extendsize = chunksize;
            if (asize < (chunksize >> 1))
                // request smaller chunk, so dynamically decrease chunksize by factor of 2
                chunksize = max(min_chunksize, chunksize >> 1);
        }

        block = extend_heap(extendsize);
        // extend_heap returns an error
        if (block == NULL) {
            return bp;
        }
    }

    place(block, asize);
    bp = header_to_payload(block);

    dbg_printf("Malloc size %zd on address %p, with adjusted size %zd.\n", size, bp, asize);
    print_heap();
    assert(checkheap(__LINE__));
    return bp;
}

/*
 * free
 * 
 * quote from textbook:
 *      To free a block, we coalesce and place the result on the appropriate free list.
 */
void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    assert(checkheap(__LINE__));
    dbg_printf("free %p\n", ptr);

    block_t *block = payload_to_header(ptr);

    mark_as_free(block);

    coalesce(block);
    assert(checkheap(__LINE__));
}

/*
 * realloc
 */
void *realloc(void *oldptr, size_t size) {
    if (size == 0) {
        free(oldptr);
        return NULL;
    }

    if (oldptr == NULL) {
        return malloc(size);
    }

    block_t *old_block = payload_to_header(oldptr);
    size_t oldsize = get_size(old_block);

    size_t asize = get_asize(size);

    dbg_printf("realloc @ %p, oldsize: %zd, newsize: %zd\n", old_block, oldsize, asize);

    // enough space to realloc, just shrink old space, and free any unused space
    if (oldsize >= asize) {
        // unused space is able to meet another malloc request, so put it into free list
        if ((oldsize - asize) >= min_block_size) {
            // maintain `prev_alloc` unchanged
            write_header(old_block, asize, true, get_prev_alloc(old_block));

            block_t *block_next = find_next(old_block);
            // the `prev_alloc` of next block is certainly true
            write_header(block_next, (oldsize - asize), false, true);
            write_footer(block_next, (oldsize - asize), false, true);

            // with newly freed block, need to coalesce
            coalesce(find_next(old_block));
        }
        return oldptr;
    }
    // old space is too small, need to alloc another space
    else {
        void *newptr;
        if ((newptr = malloc(size)) == NULL)
            return NULL;

        memcpy(newptr, oldptr, get_size(old_block) - wsize);
        free(oldptr);

        return newptr;
    }
}

/*
 * calloc
 */
void *calloc(size_t nmemb, size_t size) {
    size_t bytes = nmemb * size;
    void *newptr;

    if ((newptr = malloc(bytes)) != NULL)
        memset(newptr, 0, bytes);
    return newptr;
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

    block_t *old_epi = (block_t *)(((char *)bp) - epilogue_size);
    /**
     * Initialize free block header/footer at the position of old epilogue
     * need to maintain `prev_alloc` bit
     */
    write_header(old_epi, size, false, get_prev_alloc(old_epi));
    write_footer(old_epi, size, false, get_prev_alloc(old_epi));

    // Create new epilogue header
    block_t *new_epi = find_next(old_epi);
    // `prev_alloc` is certainly false
    write_header(new_epi, 0, true, false);

    // Coalesce in case the previous block was free
    return coalesce(old_epi);
}

/* Coalesce: Coalesces current block with previous and next blocks if either
 *           or both are unallocated; otherwise the block is not modified.
 *           Returns pointer to the coalesced block. After coalescing, the
 *           immediate contiguous previous and next blocks must be allocated.
 * 
 * need to maintain the explicit free list, remove the coalesced free block if necessary
 * and insert the newly free block
 * 
 * also need to care about `prev_alloc`
 */
static block_t *coalesce(block_t *block) {
    block_t *block_next = find_next(block);

    bool next_alloc = get_alloc(block_next);
    // prev_alloc is get from current block header
    bool prev_alloc = get_prev_alloc(block);

    size_t size = get_size(block);

    block_t *block_prev;

    // Case 1
    if (prev_alloc && next_alloc) {
        // mark the `prev_alloc` bit of next block as free
        mark_prev_alloc_as_free(block_next);
    }
    // Case 2
    else if (prev_alloc && !next_alloc) {
        size += get_size(block_next);
        write_header(block, size, false, true);
        write_footer(block, size, false, true);

        remove_from_free_list(block_next);
    }
    // Case 3
    else if (!prev_alloc && next_alloc) {
        block_prev = find_prev(block);

        size += get_size(block_prev);
        write_header(block_prev, size, false, true);
        write_footer(block_prev, size, false, true);
        block = block_prev;

        remove_from_free_list(block_prev);

        // mark the `prev_alloc` bit of next block as free
        mark_prev_alloc_as_free(block_next);
    }
    // Case 4
    else {
        block_prev = find_prev(block);

        size += get_size(block_next) + get_size(block_prev);
        write_header(block_prev, size, false, true);
        write_footer(block_prev, size, false, true);
        block = block_prev;

        remove_from_free_list(block_prev);
        remove_from_free_list(block_next);
    }

    insert_free_block(block);

    dbg_printf("After coalesce\n");
    print_heap();

    return block;
}

/*
 * place: Places block with size of asize at the start of bp. If the remaining
 *        size is at least the minimum block size, then split the block to the
 *        the allocated block and the remaining block as free.
 * 
 * need to maintain explicit free list, remove this allocated block,
 * and insert the remaining free block if necessary
 * 
 * also need to mark `prev_alloc` bit
 */
static void place(block_t *block, size_t asize) {
    dbg_printf("place %zd bytes at %p\n", asize, get_header(block));
    size_t csize = get_size(block);

    remove_from_free_list(block);

    if ((csize - asize) >= min_block_size) {
        split_block(block, asize);
    } else {
        // `prev_alloc` of block is certainly true
        write_header(block, csize, true, true);
        // `prev_alloc` of next block need to marked as alloced
        mark_prev_alloc_as_alloced(find_next(block));
    }
}

/*
 * find_fit: Looks for a free block with at least asize bytes with
 *           first-fit policy. Returns NULL if none is found.
 */
static block_t *find_fit(size_t asize) {
    // determine the size class of the request `asize`
    uint32_t idx = get_seglist_idx(asize);

    block_t *block;
    // repeat until we find a block that fits
    while (idx < size_class_cnt) {
        // check each free block in this size class
        for (block = get_block_ptr_by_offset(seglist[idx].succ_offset);
             // not until loop back to the header of this size class
             block != &(seglist[idx]);
             block = get_block_ptr_by_offset(block->succ_offset)) {
            // only need to check size, since all blocks here are free
            if (asize <= get_size(block)) {
                return block;
            }
        }
        idx++;
    }

    // or none of the free lists yields a block that fits
    return NULL;
}

/*
 * Insert new alloced block to the corresponding free list, LIFO style
 */
static void insert_free_block(block_t *bp) {
    // determine the size class of the request
    uint32_t idx = get_seglist_idx(get_size(bp));
    /**
     * pred point to the size class bucket
     * 
     * it is LIFO style: 
     *  put the newly freed block to the head of the explicit free list
     */
    bp->pred_offset = get_offset_by_block_ptr(&(seglist[idx]));
    bp->succ_offset = seglist[idx].succ_offset;
    (get_block_ptr_by_offset(bp->pred_offset))->succ_offset = get_offset_by_block_ptr(bp);
    (get_block_ptr_by_offset(bp->succ_offset))->pred_offset = get_offset_by_block_ptr(bp);
}

/*
 * Remove this block from free list
 */
static void remove_from_free_list(block_t *bp) {
    (get_block_ptr_by_offset(bp->pred_offset))->succ_offset = bp->succ_offset;
    (get_block_ptr_by_offset(bp->succ_offset))->pred_offset = bp->pred_offset;
}

/**
 * Split block by `asize`
 * 
 * the first splitted block is allocated, 
 * the second one is free,
 * need to maintain the explicit free list: insert the second one to free list
 * 
 * also need to maintain the `prev_alloc`
 */
static void split_block(block_t *bp, size_t asize) {
    size_t csize = get_size(bp);
    // maintain `prev_alloc` unchanged
    write_header(bp, asize, true, get_prev_alloc(bp));

    block_t *block_next = find_next(bp);
    // the `prev_alloc` is certainly true
    write_header(block_next, (csize - asize), false, true);
    write_footer(block_next, (csize - asize), false, true);
    insert_free_block(block_next);

    dbg_printf("Split block, first at %p, second at %p\n", bp, block_next);
}

/**
 * calculate the corresponding index of a size class for which `size` can fit in its size range
 */
static uint32_t get_seglist_idx(size_t size) {
    if (size <= min_block_size)
        return 0;
    else if (size <= 32)
        return ((size - 1) >> 3) - 1;
    else if (size <= 64)
        return ((size - 1) >> 4) + 1;
    else {
        uint32_t highest_1bit_idx = get_highest_1bit_idx(size - 1);

        uint32_t idx = min(highest_1bit_idx - 2, size_class_cnt - 1);

        // make sure: idx < 16
        dbg_ensures(idx < size_class_cnt);
        return idx;
    }
}

/**
 * calculate the index of highest `1` in the bit pattern of `num`
 * 
 * use builtin function to fast calculate
 * ref: `__builtin_clz` from https://gcc.gnu.org/onlinedocs/gcc-4.8.0/gcc/Other-Builtins.html
 */
#ifdef USE_GCC_BUILTIN
static uint32_t get_highest_1bit_idx(uint32_t num) {
    dbg_assert(num > 0);
    return 31 - __builtin_clz(num);
}
#endif

/**
 * calculate the index of highest `1` in the bit pattern of `num`
 */
#ifndef USE_GCC_BUILTIN
static uint32_t get_highest_1bit_idx(uint32_t num) {
    dbg_assert(num > 0);
    uint8_t hi = 31, lo = 0, width = 16;
    uint32_t idx = 0;
    while (width) {
        if ((num >> width)) {
            lo += width;
            idx += width;
            num >>= width;
        } else {
            hi -= width;
        }

        if (num >= (uint32_t)(1 << (width - 1))) {
            idx += (width - 1);
            break;
        }
        width >>= 1;
    }

    return idx + 1;
}
#endif

#ifndef USE_MACRO
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
    return asize - dsize;
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
 * extract_prev_alloc: returns the previous block allocation status of a given header value based
 *                on the header specification above.
 */
static bool extract_prev_alloc(word_t word) {
    return (bool)(word & prev_alloc_mask);
}

/*
 * get_prev_alloc: returns true when the previous block is allocated based on the
 *            block header's lowest bit, and false otherwise.
 */
static bool get_prev_alloc(block_t *block) {
    return extract_prev_alloc(block->header);
}

/*
 * write_header: given a block, its size, its allocation status, the allocation status of its previous block
 *               writes an appropriate value to the block header.
 */
static void write_header(block_t *block, size_t size, bool alloc, bool prev_alloc) {
    block->header = pack(size, alloc, prev_alloc);
}

/*
 * write_footer: given a block, its size, its allocation status, the allocation status of its previous block
 *               writes an appropriate value to the block footer by first
 *               computing the position of the footer.
 */
static void write_footer(block_t *block, size_t size, bool alloc, bool prev_alloc) {
    word_t *footer_ptr = get_footer(block);
    *footer_ptr = pack(size, alloc, prev_alloc);
}

/**
 * mark the previous block allocation status of this block as free
 * 
 * set the second lowest bit of header as 0
 */
static void mark_prev_alloc_as_free(block_t *block) {
    block->header &= (~0x2);
}

/**
 * mark the previous block allocation status of this block as allocated
 *
 * set the second lowest bit of header as 1
 */
static void mark_prev_alloc_as_alloced(block_t *block) {
    block->header |= 0x02;
}

/**
 * mark the allocation status of this block as free
 * both header and footer
 */
static void mark_as_free(block_t *block) {
    block->header &= (~alloc_mask);
    word_t *footer_ptr = get_footer(block);
    *footer_ptr = block->header;
}

/*
 * get_header: return the block header address
 */
static word_t *get_header(block_t *block) {
    return &(block->header);
}

/*
 * get_footer: return the block footer address
 */
static word_t *get_footer(block_t *block) {
    return (word_t *)((block->payload) + get_payload_size(block));
}

/*
 * find_next: returns the next consecutive block on the heap by adding the
 *            size of the block.
 */
static block_t *find_next(block_t *block) {
    return (block_t *)(((char *)block) + get_size(block));
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
    word_t *footer_ptr = find_prev_footer(block);
    size_t size = extract_size(*footer_ptr);
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
 * Adjust block size to include overhead and to meet alignment requirements
 */
static size_t get_asize(size_t size) {
    return round_up(size + dsize, dsize);
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
 * pack: returns a header reflecting a specified size, its alloc status, its previous block all status.
 *       If the block is allocated, the lowest bit is set to 1, and 0 otherwise.
 *       If the previous block is allocated, the second lowest bit is set to 1, and 0 otherwise.
 */
static word_t pack(size_t size, bool alloc, bool prev_alloc) {
    return size | alloc | (prev_alloc ? 2 : 0);
}

/**
 * get block pointer by adding offset to heap_start
 */
static block_t *get_block_ptr_by_offset(word_t offset) {
    return (block_t *)(heap_start + offset);
}

/**
 * get block offset by subtracting block pointer from heap_start
 */
static word_t get_offset_by_block_ptr(block_t *bp) {
    return (word_t)(((char *)bp) - heap_start);
}
#endif

/*
 * mm_checkheap
 * 
 * - 
 * -
 */
bool mm_checkheap(int lineno) {
    if (!lineno)
        return true;

    if (!heap_listp) {
        printf("NULL heap list pointer!\n");
        return false;
    }

    block_t *hi = mem_heap_hi();

    block_t *prev = NULL, *curr = heap_listp, *next;
    bool prev_alloced = true, curr_alloced;
    word_t hdr, ftr;
    while (curr + 1 < hi) {
        next = find_next(curr);
        hdr = curr->header;

        if (get_prev_alloc(curr) != prev_alloced) {
            printf("Prev alloc bit %d at %p != previous block alloced status: %d at %p, lineno: %d\n",
                   get_prev_alloc(curr), get_header(curr), prev_alloced, get_header(prev), lineno);
            printf("Prev block at %p status: size: %zd, alloced: %d, prev_alloced: %d, next: %p\n",
                   get_header(prev), get_size(prev), get_alloc(prev), get_prev_alloc(prev), find_next(prev));
            printf("Curr block at %p status: size: %zd, alloced: %d, prev_alloced: %d, next: %p\n",
                   get_header(curr), get_size(curr), get_alloc(curr), get_prev_alloc(curr), find_next(curr));
            return false;
        }

        curr_alloced = get_alloc(curr);
        if (!curr_alloced) {
            /**
             * check header and footer consistency
             */
            ftr = *find_prev_footer(next);
            if (hdr != ftr) {
                printf("Header (0x%08X) at %p != footer (0x%08X) at %p, lineno: %d\n",
                       hdr, get_header(curr), ftr, get_footer(curr), lineno);
                return false;
            }
        }

        prev_alloced = curr_alloced;
        prev = curr;
        curr = next;
    }

    return true;
}

/*
 * print heap in continuous style, block by block
 * commonly show block position, header, footer
 * 
 * - for *allocated* block, show payload size
 * - for *free* block, show predecessor pointer, successor pointer
 */
static void dbg_print_heap(bool print) {
    if (!print)
        return;

    block_t *hi = mem_heap_hi();

    block_t *curr = heap_listp, *next;
    word_t hdr, ftr;
    while (curr + 1 < hi) {
        next = find_next(curr);

        hdr = curr->header;

        if (get_alloc(curr)) {
            printf("@%p->[h:%zd/%s/%s|psize:%zd] ",
                   curr, extract_size(hdr),
                   extract_prev_alloc(hdr) ? "pa" : "pf",
                   extract_alloc(hdr) ? "a" : "f",
                   get_payload_size(curr));
        } else {
            ftr = *find_prev_footer(next);
            printf("@%p->[h:%zd/%s/%s|pred:%p,succ:%p|f:%zd/%s] ",
                   curr, extract_size(hdr),
                   extract_prev_alloc(hdr) ? "pa" : "pf",
                   extract_alloc(hdr) ? "a" : "f",
                   get_block_ptr_by_offset(curr->pred_offset),
                   get_block_ptr_by_offset(curr->succ_offset),
                   extract_size(ftr),
                   extract_alloc(ftr) ? "a" : "f");
        }

        curr = next;
    }
    printf("\n");
}
