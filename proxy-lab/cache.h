#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"

typedef struct cache_item {
    char hostname[MAXLINE];
    char hostport[6];
    char path[MAXLINE];

    struct cache_item *prev;
    struct cache_item *next;

    int lru;
    size_t cache_size;
    char cache[0];
} cache_item_t;

typedef struct cache {
    cache_item_t *cache_listp;
    size_t total_size;
    int curr_lru;
} cache_t;

void cache_init(cache_t *cp);
void cache_deinit(cache_t *cp);
cache_item_t *build_cache_item(char *hostname, char *hostport, char *path, char *cache, size_t cache_size);
void cache_insert(cache_t *cp, cache_item_t *item_p);
cache_item_t *cache_find(cache_t *cp, char *hostname, char *hostport, char *path);

#endif