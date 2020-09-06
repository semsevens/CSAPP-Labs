/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <stdio.h>

#define BSIZE 8

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void swap_matrix_element(int *a, int *b) {
    *a = *a + *b;
    *b = *a - *b;
    *a = *a - *b;
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_row_wise_desc[] = "Simple row-wise scan transpose";
void trans_row_wise(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char trans_column_wise_desc[] = "Simple column-wise scan transpose";
void trans_column_wise(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (j = 0; j < M; j++) {
        for (i = 0; i < N; i++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char trans_32_square_matrix_desc[] = "Block wise transpose, especially for 32*32 matrix";
void transpose_32_square_matrix(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 32) {
        return;
    }

    int a0, a1, a2, a3, a4, a5, a6, a7;
    size_t ii, jj, i, j;
    for (ii = 0; ii < N; ii += BSIZE) {
        for (jj = ii; jj < N; jj += BSIZE) {
            // copy from A
            for (i = ii; i < ii + 8; i++) {
                j = jj;
                a0 = A[i][j], a1 = A[i][j + 1], a2 = A[i][j + 2], a3 = A[i][j + 3], a4 = A[i][j + 4], a5 = A[i][j + 5], a6 = A[i][j + 6], a7 = A[i][j + 7];
                B[i][j] = a0, B[i][j + 1] = a1, B[i][j + 2] = a2, B[i][j + 3] = a3, B[i][j + 4] = a4, B[i][j + 5] = a5, B[i][j + 6] = a6, B[i][j + 7] = a7;
            }
            if (ii != jj) {
                for (i = jj; i < jj + 8; i++) {
                    j = ii;
                    a0 = A[i][j], a1 = A[i][j + 1], a2 = A[i][j + 2], a3 = A[i][j + 3], a4 = A[i][j + 4], a5 = A[i][j + 5], a6 = A[i][j + 6], a7 = A[i][j + 7];
                    B[i][j] = a0, B[i][j + 1] = a1, B[i][j + 2] = a2, B[i][j + 3] = a3, B[i][j + 4] = a4, B[i][j + 5] = a5, B[i][j + 6] = a6, B[i][j + 7] = a7;
                }
            }

            // use block, which `bsize=8`, to transpose
            for (i = ii; i < ii + BSIZE; i++) {
                for (j = ii == jj ? i + 1 : jj; j < jj + BSIZE; j++) {
                    swap_matrix_element(&B[i][j], &B[j][i]);
                }
            }
        }
    }
}


char trans_32_square_matrix_v2_desc[] = "Block wise transpose version 2, especially for 32*32 matrix";
void transpose_32_square_matrix_v2(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 32) {
        return;
    }

    int a0, a1, a2, a3, a4, a5, a6, a7;
    size_t ii, jj, i, j;
    for (ii = 0; ii < N; ii += BSIZE) {
        for (jj = 0; jj < N; jj += BSIZE) {
            for (i = ii; i < ii + 8; i++) {
                j = jj;
                a0 = A[i][j], a1 = A[i][j + 1], a2 = A[i][j + 2], a3 = A[i][j + 3], a4 = A[i][j + 4], a5 = A[i][j + 5], a6 = A[i][j + 6], a7 = A[i][j + 7];
                B[j][i] = a0, B[j + 1][i] = a1, B[j + 2][i] = a2, B[j + 3][i] = a3, B[j + 4][i] = a4, B[j + 5][i] = a5, B[j + 6][i] = a6, B[j + 7][i] = a7;
            }
        }
    }
}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    if (M == N && N == 32) {
        transpose_32_square_matrix_v2(M, N, A, B);
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans_row_wise, trans_row_wise_desc);
    // registerTransFunction(trans_column_wise, trans_column_wise_desc);
    // registerTransFunction(transpose_32_square_matrix, trans_32_square_matrix_desc);
    // registerTransFunction(transpose_32_square_matrix_v2, trans_32_square_matrix_v2_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
