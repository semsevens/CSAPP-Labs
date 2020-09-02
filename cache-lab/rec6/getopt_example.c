#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
	
	extern char *optarg;
	extern int optind, opterr, optopt;
	
    int verbose = 0;
    int n = 0;
    char opt;

    while ((opt = getopt(argc, argv, "vn:")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            case 'n':
                n = atoi(optarg);
                break;
            default:
                fprintf(stderr, "usage: …");
                exit(1);
        }
    }

    for (int i = 0; i < n; i++) {
        if (verbose) printf("%d\n", i);
    }

    printf("Done counting to %d\n", n);

    return 0;
}