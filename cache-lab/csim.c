#define _GNU_SOURCE
#include "cachelab.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COUNTER_START 0

// global counter for LRU
unsigned int counter = COUNTER_START;

/**
 * struct to store cmd options
 */
typedef struct {
    bool verbose;
    unsigned int s;
    unsigned int E;
    unsigned int b;
    char *tracefile;
} cmdOpts;

/**
 * cache line
 */
typedef struct {
    bool valid;
    unsigned int tag;
    unsigned int lruCounter;
    char blocks[];
} line;

/**
 * cache set
 */
typedef struct
{
    unsigned int E;
    line *lines[];
} set;

/**
 * struct to simulate cache
 */
typedef struct {
    unsigned int hitCount;
    unsigned int missCount;
    unsigned int evictionCount;

    unsigned int s;
    unsigned int S;
    unsigned int b;
    unsigned int B;
    unsigned int E;

    set *sets[];
} cache;

/**
 * struct to represent bits segment from 64-bit address
 */
typedef struct
{
    unsigned int setIdx;
    unsigned int blockOffset;
    unsigned int tag;
} addrSegment;

/**
 * parse cmd options, print usage message if asked
 */
cmdOpts *parseCmdOpts(int argc, char *argv[]) {
    cmdOpts *opts = malloc(sizeof(cmdOpts));
    if (opts == NULL) {
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
    }

    extern char *optarg;
    extern int optind, opterr, optopt;
    char opt;

    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
        case 'v':
            opts->verbose = true;
            break;
        case 's':
            opts->s = atoi(optarg);
            break;
        case 'E':
            opts->E = atoi(optarg);
            break;
        case 'b':
            opts->b = atoi(optarg);
            break;
        case 't':
            opts->tracefile = optarg;
            break;
        default:
            fprintf(stderr, "usage:\
            \n\t-h: Optional help flag that prints usage info\
            \n\t-v: Optional verbose flag that displays trace info\
            \n\t-s <s>: Number of set index bits (S = 2 s is the number of sets)\
            \n\t-E <E>: Associativity (number of lines per set)\
            \n\t-b <b>: Number of block bits (B = 2 b is the block size)\
            \n\t-t <tracefile>: Name of the valgrind trace to replay\n");
            exit(EXIT_SUCCESS);
        }
    }

    return opts;
}

/**
 * initialize a cache line
 */
line *initLine(unsigned int B) {
    line *l = malloc(sizeof(line) + sizeof(char) * B);
    if (l == NULL) {
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
    }
    l->valid = false;
    l->lruCounter = COUNTER_START;

    return l;
}

/**
 * initialize a cache set
 */
set *initSet(unsigned int E, unsigned int B) {
    set *s = malloc(sizeof(set) + sizeof(line*) * E);
    if (s == NULL) {
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
    }
    s->E = E;

    for (size_t i = 0; i < E; i++) {
        s->lines[i] = initLine(B);
    }

    return s;
}

/**
 * initialize a cache
 */
cache *init(cmdOpts *opts) {
    unsigned int S = 1 << opts->s;
    unsigned int B = 1 << opts->b;

    cache *c = malloc(sizeof(cache) + sizeof(set*) * S);
    if (c == NULL) {
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
    }
    c->hitCount = c->missCount = c->evictionCount = 0;
    c->s = opts->s;
    c->S = S;
    c->b = opts->b;
    c->B = B;
    c->E = opts->E;

    for (size_t i = 0; i < S; i++) {
        c->sets[i] = initSet(opts->E, B);
    }

    return c;
}
/**
 * segment 64-bit address into three parts: set index bits, block bits, tag bits
 */
addrSegment *segmentAddress(unsigned long address, unsigned int s, unsigned int b) {
    addrSegment *seg = malloc(sizeof(addrSegment));
    if (seg == NULL) {
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
    }

    seg->blockOffset = address & ((1 << b) - 1);
    seg->setIdx = (address & ((1 << (b + s)) - 1)) >> b;
    seg->tag = address >> (b + s);

    return seg;
}

void load(unsigned long address, cache *cache) {
    addrSegment *seg = segmentAddress(address, cache->s, cache->b);

    set *set = cache->sets[seg->setIdx];
    int hitLineIdx = -1;
    unsigned int evictLineIdx = 0;
    unsigned int evictLruCounter = set->lines[evictLineIdx]->lruCounter;
    for (size_t i = 0; i < cache->E; i++) {
        if (set->lines[i]->valid && set->lines[i]->tag == seg->tag) {
            hitLineIdx = i;
            break;
        }
        if (set->lines[i]->lruCounter < evictLruCounter) {
            evictLineIdx = i;
            evictLruCounter = set->lines[i]->lruCounter;
        }
    }

    if (hitLineIdx >= 0) {
        cache->hitCount++;
        set->lines[hitLineIdx]->lruCounter = counter;
    } else {
        cache->missCount++;
        if (evictLruCounter != COUNTER_START) {
            cache->evictionCount++;
        }

        set->lines[evictLineIdx]->valid = true;
        set->lines[evictLineIdx]->tag = seg->tag;
        set->lines[evictLineIdx]->lruCounter = counter;
    }

    free(seg);
    return;
}

void store(unsigned long address, cache *cache) {
    addrSegment *seg = segmentAddress(address, cache->s, cache->b);

    free(seg);
    return;
}

/**
 * simulate cache behaviour for each line
 */
void simulate(char *line, cache *cache) {
    if (line[0] == 'I') {
        return;
    }

    char opType = line[1];
    char *addressStr = strtok(line + 3, ",");
    char *end;
    unsigned long address = strtoul(addressStr, &end, 16);

    switch (opType) {
    case 'L':
        load(address, cache);
        break;
    case 'S':
        load(address, cache);
        break;
    case 'M':
        load(address, cache);
        load(address, cache);
        break;
    default:
        break;
    }
}

void freeCache(cache *cache) {
    for (size_t i = 0; i < cache->S; i++) {
        for (size_t j = 0; j < cache->E; j++) {
            free(cache->sets[i]->lines[j]);
        }
        free(cache->sets[i]);
    }

    free(cache);
}

int main(int argc, char *argv[]) {
    cmdOpts *opts = parseCmdOpts(argc, argv);
    cache *cache = init(opts);
    free(opts);

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(opts->tracefile, "r");
    if (fp == NULL) {
        fprintf(stderr, "open file: %s failed", opts->tracefile);
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        counter++;
        simulate(line, cache);
    }
    fclose(fp);
    free(line);

    printSummary(cache->hitCount, cache->missCount, cache->evictionCount);

    freeCache(cache);
    return 0;
}
