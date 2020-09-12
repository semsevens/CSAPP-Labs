#define _GNU_SOURCE
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int M = 0, N = 0;
    long A = 0, B = 0;
    char opt;
    while ((opt = getopt(argc, argv, "M:N:A:B:")) != -1) {
        switch (opt) {
        case 'M':
            M = atoi(optarg);
            break;
        case 'N':
            N = atoi(optarg);
            break;
        case 'A':
            A = strtoul(optarg, NULL, 16);
            break;
        case 'B':
            B = strtoul(optarg, NULL, 16);
            break;
        default:
            break;
        }
    }

    if (!(M && N && A && B)) {
        printf("must pass M, N, A, B as arguments\n");
        exit(EXIT_FAILURE);
    }

    unsigned int A_row_size = sizeof(int) * M, B_row_size = sizeof(int) * N;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned long address;
    unsigned int i, j;
    while ((read = getline(&line, &len, stdin)) != -1) {
        line[strcspn(line, "\n")] = '\0';
        address = strtoul(line + 2, NULL, 16);

        printf("%s ", line);

        if (address >= A && address < A + A_row_size * N) {
            i = (address - A) / A_row_size;
            j = ((address - A) % A_row_size) / sizeof(int);
            printf("A[%d][%d]", i, j);
        } else if (address >= B && address < B + B_row_size * M) {
            i = (address - B) / B_row_size;
            j = ((address - B) % B_row_size) / sizeof(int);
            printf("B[%d][%d]", i, j);
        }

        printf("\n");
    }
}
