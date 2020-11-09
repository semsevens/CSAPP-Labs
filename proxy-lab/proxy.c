#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
int parse_uri(int fd, char *uri, char *hostname, char *hostport, char *path);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usaage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int listenfd = Open_listenfd(argv[1]);

    int connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char hostname[MAXLINE], port[MAXLINE];
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        dbg_printf("Accepted connection from (%s, %s)\n", hostname, port);

        doit(connfd);

        Close(connfd);
    }

    return 0;
}

void doit(int connfd) {
    rio_t conn_rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

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
    dbg_printf("hostname: %s, hostport: %s, path: %s\n", hostname, hostport, path);

    int clientfd = Open_clientfd(hostname, hostport);
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

    ssize_t readNum;
    while ((readNum = Rio_readnb(&client_rio, buf, MAXLINE))) {
        Rio_writen(connfd, buf, readNum);
    }
}

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