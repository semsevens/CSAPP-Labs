/*
 * This file is useful for generating a trace.  After making
 * a dynamic executable for this, LD_PRELOAD it and run your program.
 * It will dump the malloc/free trace into a file as directed by
 * the GENERATE_TRACE_OUTPUT envvar.
 *
 * Each pointer is labeled with its address and its PID.
 * A null pointer is labeled just as 0.
 */

#define _GNU_SOURCE
#include <sys/types.h>

#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>

static char file_prefix[] = ""
"# Memory trace collected by get-trace.o\n"
"# Use produce-simple-trace to translate to the format\n"
"# used by the driver.\n"
"# Format:\n"
"# m p u     => p = malloc(u);\n"
"# c p u1 u2 => p = calloc(u1,u2);\n"
"# r p1 p2 u => p1 = realloc(p2, u);\n"
"# f p       => free(p);\n"
"\n";

static double memory[1<<26]; /* a short execution will work */
static int myheapend = 0;

static int outfd = -1;

#define BUFFER_LEN 4000
typedef char myline[64];
static myline buffer[BUFFER_LEN];
static int nbuffered = 0;

static void clear_buffer(void) {
    int i;

    if(outfd == -1) {
        const char *outname;
        char namebuf[4096];
        char linkbuf[4096];
        int trynum = 0;
        int len;

        outname = getenv("GENERATE_TRACE_OUTPUT");
        if(!outname) {
            outname = "/tmp/trace";
        }

        /* get the executable name (on linux <= 2.0 we get the inode number;
           on other platforms we may just fail) */
        sprintf(namebuf, "/proc/%u/exe", (unsigned) getpid());
        len = readlink(namebuf, linkbuf, sizeof(linkbuf));
        if(len >= 0) {
            int i;
            for(i = 0; i < len; i++) {
                if(linkbuf[i] == '/') linkbuf[i] = '_';
            }
            linkbuf[len] = 0;
            sprintf(namebuf, "%s.%s", outname, linkbuf);
        }

        outfd = open(namebuf, O_CREAT | O_EXCL | O_WRONLY, 0666);
        if(outfd < 0) {
            char basename[4096];
            strcpy(basename, namebuf);
            do {
                assert(trynum < 10000);
                sprintf(namebuf, "%s.%d", basename, ++trynum);
                outfd = open(namebuf, O_CREAT | O_EXCL | O_WRONLY, 0666);
            } while(outfd < 0);
        }
        fcntl(outfd, F_SETFD, 1); /* close-on-exec */

        write(outfd, file_prefix, strlen(file_prefix));
    }

    for(i = 0; i < nbuffered; i++) {
        write(outfd, buffer[i], strlen(buffer[i]));
    }
    nbuffered = 0;
}

static void clear_buffer_and_close(void) {
    clear_buffer();
    close(outfd);
}


static void mywrite(const char *fmt, ...) {
    char buf[256];
    va_list ap;
    int len;
    int i;

    va_start(ap, fmt);
    len = vsprintf(buf, fmt, ap);
    va_end(ap);
    assert(len >= 0);

    for(i = 0; i <= len; i++) {
        buffer[nbuffered][i] = buf[i];
    }
    nbuffered++;
    if(nbuffered == BUFFER_LEN) {
        clear_buffer();
    }
}

static const char *print_ptr(const void *p) {
    /* realloc needs to print two pointers */
    static char buf1[256];
    static char buf2[256];
    static char *which = buf1;
    char *mine = which;

    if(p==0) {
        sprintf(mine, "0");
    } else {
        sprintf(mine, "%p/%u", p, (unsigned) getpid());
    }
    which = (which == buf1) ? buf2 : buf1;
    return mine;
}

static void init(void) {
    atexit(clear_buffer_and_close);
}

void *malloc(size_t sz) {
    if(myheapend==0) init();

    void *p = &memory[myheapend];
    myheapend += ((sz-1) / sizeof(*memory)) + 1;

    mywrite("m %s %u\n", print_ptr(p), sz);
    return p;
}

void *calloc(size_t sz1, size_t sz2) {
    if(myheapend==0) init();

    size_t sz = sz1 * sz2;
    char *p = (void*)&memory[myheapend];
    int i;
    myheapend += ((sz-1) / sizeof(*memory)) + 1;
    for(i = 0; i < sz; i++) p[i] = 0;

    mywrite("c %s %u %u\n", print_ptr(p), sz1, sz2);
    return p;
}

void *realloc(void *oldp, size_t sz) {
    if(myheapend==0) init();


    char *p;
    if(sz == 0) {
        p = 0;
    } else {
        p = (void*)&memory[myheapend];
        myheapend += ((sz-1) / sizeof(*memory)) + 1;
    }

    if(sz && oldp) {
        int i;
        memcpy(p, oldp, sz); /* clobbers after p; not a problem */
        for(i = 0; i < sz; i++) p[i] = ((char*)oldp)[i];
    }

    mywrite("r %s %s %u\n", print_ptr(p), print_ptr(oldp), sz);
    return p;
}

void free(void *p) {
    if(myheapend==0) init();
    mywrite("f %s\n", print_ptr(p));
}
