/* Recommended max cache and object sizes */
#ifndef __PROXY_H__
#define __PROXY_H__

#ifndef MAX_CACHE_SIZE
#define MAX_CACHE_SIZE 1049000
#endif

#ifndef MAX_OBJECT_SIZE
#define MAX_OBJECT_SIZE 102400
#endif

#define NTHREADS 4
#define SBUFSIZE 16

#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

#endif