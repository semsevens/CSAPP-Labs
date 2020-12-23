#include "proxy.h"
#include "cache.h"
#include "csapp.h"
#include "rwqueue.h"
#include "sbuf.h"

void *thread(void *vargp);
void doit(int fd);
void direct_serve(int connfd, char *hostname, char *hostport, char *path, char *method);
void cache_serve(int connfd, cache_item_t *cached);
int parse_uri(int fd, char *uri, char *hostname, char *hostport, char *path);

void sigint_handler(int sig);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void client500error(int fd);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
/* Shared buffer of connected descriptors */
sbuf_t sbuf;
/* Web content cache, shared by all connections */
cache_t cache;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usaage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int listenfd = Open_listenfd(argv[1]);

    // capture SIGINT, and do some clean job before exit
    Signal(SIGINT, sigint_handler);

    /*
     * multi-thread, by using producer-consumer model
     * pre-threaded
     */
    sbuf_init(&sbuf, SBUFSIZE);
    pthread_t tid;
    for (size_t i = 0; i < NTHREADS; i++) {
        Pthread_create(&tid, NULL, thread, NULL);
    }

    // init cache
    cache_init(&cache);

    int connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char hostname[MAXLINE], port[MAXLINE];
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        dbg_printf("Accepted connection from (%s, %s)\n", hostname, port);

        // produce connection
        sbuf_insert(&sbuf, connfd);
    }

    return 0;
}

void *thread(void *vargp) {
    Pthread_detach(Pthread_self());
    while (1) {
        // consume connection
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int connfd) {
    rio_t conn_rio;
    char buf[MAXLINE], method[16], uri[MAXLINE], version[16];

    Rio_readinitb(&conn_rio, connfd);
    if (!Rio_readlineb(&conn_rio, buf, MAXLINE))
        return;

    dbg_printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(connfd, method, "501", "Not Implemented", "Proxy server does not implement this method");
        return;
    }

    char hostname[MAXLINE], path[MAXLINE], hostport[6];
    if (parse_uri(connfd, uri, hostname, hostport, path) < 0) {
        return;
    };

    cache_item_t *cached = cache_find(&cache, hostname, hostport, path);
    if (!cached) {
        dbg_printf("direct serve: hostname: %s, hostport: %s, path: %s\n", hostname, hostport, path);
        direct_serve(connfd, hostname, hostport, path, method);
    } else {
        dbg_printf("serve from cache: hostname: %s, hostport: %s, path: %s\n", hostname, hostport, path);
        cache_serve(connfd, cached);
    }
}

/**
 * serve client request by proxying to target
 */
void direct_serve(int connfd, char *hostname, char *hostport, char *path, char *method) {
    char buf[MAXLINE];

    int clientfd;
    if ((clientfd = open_clientfd(hostname, hostport)) < 0) {
        client500error(connfd);
        return;
    }

    rio_t client_rio;
    Rio_readinitb(&client_rio, clientfd);
    // HTTP first line
#pragma GCC diagnostic ignored "-Wformat-overflow="
    sprintf(buf, "%s %s HTTP/1.0\r\n", method, path);
    Rio_writen(clientfd, buf, strlen(buf));
    // HTTP headers
    sprintf(buf, "Host: %s\r\n", hostname);
    Rio_writen(clientfd, buf, strlen(buf));
    sprintf(buf, "User-Agent: %s\r\n", user_agent_hdr);
    Rio_writen(clientfd, buf, strlen(buf));
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(clientfd, buf, strlen(buf));
    sprintf(buf, "Proxy-Connection: close\r\n\r\n");
    Rio_writen(clientfd, buf, strlen(buf));
#pragma GCC diagnostic pop

    char *obj_cache_base_p = Malloc(MAX_OBJECT_SIZE);
    char *obj_cache_p = obj_cache_base_p;
    ssize_t readNum, totalNum = 0;
    while ((readNum = Rio_readnb(&client_rio, buf, MAXLINE))) {
        if ((totalNum + readNum) <= MAX_OBJECT_SIZE) {
            memcpy(obj_cache_p, buf, readNum);
            obj_cache_p += readNum;
        }
        totalNum += readNum;

        Rio_writen(connfd, buf, readNum);
    }

    // object can be cached
    if (totalNum <= MAX_OBJECT_SIZE) {
        dbg_printf("%s:%s%s can be cached, object size: %zd\n", hostname, hostport, path, totalNum);
        cache_item_t *obj = build_cache_item(hostname, hostport, path, obj_cache_base_p, totalNum);

        cache_insert(&cache, obj);
    } else {
        dbg_printf("%s:%s%s cannot be cached, object size: %zd\n", hostname, hostport, path, totalNum);
    }
    Free(obj_cache_base_p);
}

/**
 * serve client request by reading from cache
 */
void cache_serve(int connfd, cache_item_t *cached) {
    Rio_writen(connfd, cached->cache, cached->cache_size);
}

/**
 * split uri as three part: hostname, hostport, path
 * e.g:
 *  - uri: www.google.com -> hostname: www.google.com, hostport: 80, path: /
 *  - uri: http://www.google.com:8080 -> hostname: www.google.com, hostport: 8080, path: /
 *  - uri: http://www.google.com:8080/xyz/index.html -> hostname: www.google.com, hostport: 8080, path: /xyz/index.html
 * 
 * return:
 *  - if uri is malformatted, return -1
 *  - else return 0
 */
int parse_uri(int fd, char *uri, char *hostname, char *hostport, char *path) {
    // relative uri path
    if (uri[0] == '/') {
        strcpy(hostname, "");
        *hostport = 80;
        strcpy(path, uri);
    }
    // absolute uri path
    else {
        char *http_p = strstr(uri, "http://");
        if (http_p) {
            if (http_p != uri) {
                clienterror(fd, uri, "400", "Bad Request", "Uri not start with 'http'");
                return -1;
            }
            uri = uri + strlen("http://");
        }

        // find the sperator between host and path
        char *ptr = index(uri, '/');
        if (ptr) {
            strcpy(path, ptr);
            *ptr = '\0';
        } else {
            strcpy(path, "/");
        }
        // find the sperator between host name and host port (if exists)
        ptr = index(uri, ':');
        if (ptr) {
            strcpy(hostport, ptr + 1);
            *ptr = '\0';
        } else {
            strcpy(hostport, "80");
        }
        // the remaining is host name
        strcpy(hostname, uri);
    }
    return 0;
}

/**
 * common method to return error to client
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE];

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=\"ffffff\">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>Proxy server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

void client500error(int fd) {
    clienterror(fd, "Internal server error", "500", "Internal server error", "Internal server error");
}

/**
 * graceful shutdown handler
 */
void sigint_handler(int sig) {
    int olderrno = errno;

    Sio_puts("Gracefully shutdown...\nFreeing allocated memory...\n");
    sbuf_deinit(&sbuf);
    cache_deinit(&cache);
    Sio_puts("Freed\nShutting down....\n");

    Signal(SIGINT, SIG_DFL);
    Kill(getpid(), SIGINT);

    errno = olderrno;
}