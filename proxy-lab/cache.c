#include "cache.h"
#include "csapp.h"

void cache_init(cache_t *cp) {
    cp->cache_listp = (cache_item_t *)Malloc(sizeof(cache_item_t));
    cp->cache_listp->next = cp->cache_listp->prev = cp->cache_listp;
    cp->total_size = 0;
}

void cache_deinit(cache_t *cp) {
    cache_item_t *cache_item_p = cp->cache_listp;
    cache_item_t *next = NULL;
    while (next != cp->cache_listp) {
        next = cache_item_p->next;
        Free(cache_item_p);
        cache_item_p = next;
    }
}

cache_item_t *build_cache_item(char *hostname, char *hostport, char *path, char *cache, size_t cache_size) {
    cache_item_t *item_p = (cache_item_t *)Malloc(sizeof(cache_item_t) + cache_size);
    memcpy(item_p->hostname, hostname, strlen(hostname));
    memcpy(item_p->hostport, hostport, strlen(hostport));
    memcpy(item_p->path, path, strlen(path));

    memcpy(item_p->cache, cache, cache_size);
    item_p->cache_size = cache_size;
    item_p->lru = 1;

    return item_p;
}

void cache_insert(cache_t *cp, cache_item_t *item_p) {
    // insert into head
    item_p->next = cp->cache_listp->next;
    item_p->next->prev = item_p;
    cp->cache_listp->next = item_p;
    item_p->prev = cp->cache_listp;

    // evict
}

cache_item_t *cache_find(cache_t *cp, char *hostname, char *hostport, char *path) {
    cache_item_t *curr = cp->cache_listp->next;
    while (curr != cp->cache_listp) {
        if (!strcmp(hostname, curr->hostname) &&
            !strcmp(hostport, curr->hostport) &&
            !strcmp(path, curr->path)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}