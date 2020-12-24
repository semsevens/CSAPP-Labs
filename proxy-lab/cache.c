#include "cache.h"
#include "csapp.h"
#include "proxy.h"

/**
 * Helper routine to display cache layout in a nice way
 */
void display_cache(cache_t *cp) {
    cache_item_t *curr = cp->cache_listp->next;
    dbg_printf("in-memory cache: {%zd, %d}", cp->total_size, cp->curr_lru);
    while (curr != cp->cache_listp) {
        dbg_printf(" -> [%s, %zd, %d]", curr->path, curr->cache_size, curr->lru);
        curr = curr->next;
    }
    dbg_printf("\n");
}

/**
 * LRU eviction policy
 *
 * both reading and writing counts
 */
void set_lru(cache_t *cp, cache_item_t *item_p) {
    item_p->lru = ++(cp->curr_lru);
}

void cache_init(cache_t *cp) {
    cp->cache_listp = (cache_item_t *)Malloc(sizeof(cache_item_t));
    cp->cache_listp->next = cp->cache_listp->prev = cp->cache_listp;
    cp->total_size = 0;
    cp->curr_lru = 0;
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
    memcpy(item_p->hostname, hostname, strlen(hostname) + 1);
    memcpy(item_p->hostport, hostport, strlen(hostport) + 1);
    memcpy(item_p->path, path, strlen(path) + 1);

    memcpy(item_p->cache, cache, cache_size);
    item_p->cache_size = cache_size;
    item_p->lru = 0;

    return item_p;
}

static void remove_cache_item(cache_t *cp, cache_item_t *item_p) {
    dbg_printf("evict item: %s:%s%s\n", item_p->hostname, item_p->hostport, item_p->path);
    item_p->prev->next = item_p->next;
    item_p->next->prev = item_p->prev;

    cp->total_size -= item_p->cache_size;

    Free(item_p);
}

static void evict(cache_t *cp, size_t required_size) {
    cache_item_t *victim_item_p, *curr;
    int min_lru;
    while (cp->total_size + required_size > MAX_CACHE_SIZE) {
        curr = cp->cache_listp->next;
        min_lru = cp->curr_lru;

        while (curr != cp->cache_listp) {
            if (curr->lru < min_lru) {
                min_lru = curr->lru;
                victim_item_p = curr;
            }
            curr = curr->next;
        }

        dbg_printf("Before evict: ");
        display_cache(cp);
        remove_cache_item(cp, victim_item_p);
        dbg_printf("After evict: ");
        display_cache(cp);
    }
}

void cache_insert(cache_t *cp, cache_item_t *item_p) {
    dbg_printf("Before insert: ");
    display_cache(cp);

    // evict if necessary
    if (cp->total_size + item_p->cache_size > MAX_CACHE_SIZE) {
        evict(cp, item_p->cache_size);
    }
    cp->total_size += item_p->cache_size;

    // insert into head
    item_p->next = cp->cache_listp->next;
    item_p->next->prev = item_p;
    cp->cache_listp->next = item_p;
    item_p->prev = cp->cache_listp;

    set_lru(cp, item_p);

    dbg_printf("After insert: ");
    display_cache(cp);
}

cache_item_t *cache_find(cache_t *cp, char *hostname, char *hostport, char *path) {
    cache_item_t *curr = cp->cache_listp->next;
    while (curr != cp->cache_listp) {
        if (!strcmp(hostname, curr->hostname) &&
            !strcmp(hostport, curr->hostport) &&
            !strcmp(path, curr->path)) {
            set_lru(cp, curr);
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}