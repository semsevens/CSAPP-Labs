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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void swap_matrix_element(int *a, int *b) {
    *a = *a + *b;
    *b = *a - *b;
    *a = *a - *b;
}

void trans_bsize_8(int i, int j, int M, int N, int A[N][M], int B[M][N]) {
    int a0 = A[i][j], a1 = A[i][j + 1], a2 = A[i][j + 2], a3 = A[i][j + 3], a4 = A[i][j + 4], a5 = A[i][j + 5], a6 = A[i][j + 6], a7 = A[i][j + 7];
    B[j][i] = a0, B[j + 1][i] = a1, B[j + 2][i] = a2, B[j + 3][i] = a3, B[j + 4][i] = a4, B[j + 5][i] = a5, B[j + 6][i] = a6, B[j + 7][i] = a7;
}

void trans_bsize_4(int i, int j, int M, int N, int A[N][M], int B[M][N]) {
    int a0 = A[i][j], a1 = A[i][j + 1], a2 = A[i][j + 2], a3 = A[i][j + 3];
    B[j][i] = a0, B[j + 1][i] = a1, B[j + 2][i] = a2, B[j + 3][i] = a3;
}

void copy_bsize_8(int i, int j, int M, int N, int A[N][M], int B[M][N]) {
    int a0 = A[i][j], a1 = A[i][j + 1], a2 = A[i][j + 2], a3 = A[i][j + 3], a4 = A[i][j + 4], a5 = A[i][j + 5], a6 = A[i][j + 6], a7 = A[i][j + 7];
    B[i][j] = a0, B[i][j + 1] = a1, B[i][j + 2] = a2, B[i][j + 3] = a3, B[i][j + 4] = a4, B[i][j + 5] = a5, B[i][j + 6] = a6, B[i][j + 7] = a7;
}

void trans_bsize_2x4(int i, int j, int M, int N, int A[N][M], int B[M][N]) {
    int a00 = A[i][j], a01 = A[i][j + 1], a02 = A[i][j + 2], a03 = A[i][j + 3],
        a10 = A[i + 1][j], a11 = A[i + 1][j + 1], a12 = A[i + 1][j + 2], a13 = A[i + 1][j + 3];
    B[j][i] = a00, B[j][i + 1] = a10,
    B[j + 1][i] = a01, B[j + 1][i + 1] = a11,
    B[j + 2][i] = a02, B[j + 2][i + 1] = a12,
    B[j + 3][i] = a03, B[j + 3][i + 1] = a13;
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

char trans_32_square_matrix_desc[] = "Block wise transpose, only for 32*32 matrix";
void transpose_32_square_matrix(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 32) {
        return;
    }

    const int BSIZE = 8;

    size_t ii, jj, i, j;
    for (ii = 0; ii < N; ii += BSIZE) {
        for (jj = ii; jj < N; jj += BSIZE) {
            // copy from A
            for (i = ii; i < ii + 8; i++) {
                j = jj;
                copy_bsize_8(i, j, M, N, A, B);
            }
            if (ii != jj) {
                for (i = jj; i < jj + 8; i++) {
                    j = ii;
                    copy_bsize_8(i, j, M, N, A, B);
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

char trans_32_square_matrix_v2_desc[] = "Block wise transpose version 2, only for 32*32 matrix";
void transpose_32_square_matrix_v2(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 32) {
        return;
    }

    const int BSIZE = 8;

    // block index
    size_t bi, bj;
    // inner-block index
    size_t i, j;

    for (bi = 0; bi < N; bi += BSIZE) {
        for (bj = 0; bj < N; bj += BSIZE) {
            for (i = bi; i < bi + BSIZE; i++) {
                j = bj;
                trans_bsize_8(i, j, M, N, A, B);
            }
        }
    }
}

char trans_64_square_matrix_desc[] = "Block wise transpose, with zig-zag access pattern, only for 64*64 matrix";
void transpose_64_square_matrix(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 64) {
        return;
    }

    const int BSIZE = 4;

    // zig-zag index
    size_t zzi, zzj;
    // block count in a zig-zag pattern
    int zbc;
    // block index in a zig-zag pattern
    size_t bi;
    // block upper left index (i, j)
    size_t bul_i, bul_j;
    // inner-block index
    size_t i, j;

    for (zzi = 0, zbc = 1; zzi < N; zzi += BSIZE, zbc++) {
        for (bi = 0; bi < zbc; bi++) {
            bul_i = zzi - bi * BSIZE, bul_j = 0 + bi * BSIZE;

            for (i = bul_i; i < bul_i + BSIZE; i++) {
                j = bul_j;
                trans_bsize_4(i, j, M, N, A, B);
            }
        }
    }

    // cross anti-diagonal, so zig-zag block count will decrease one by one
    zbc--;

    for (zzj = BSIZE; zzj < N; zzj += BSIZE, zbc--) {
        for (bi = 0; bi < zbc; bi++) {
            bul_i = N - (bi + 1) * BSIZE, bul_j = zzj + bi * BSIZE;

            for (i = bul_i; i < bul_i + BSIZE; i++) {
                j = bul_j;
                trans_bsize_4(i, j, M, N, A, B);
            }
        }
    }
}

char trans_64_square_matrix_v2_desc[] = "Block wise transpose, with zig-zag access pattern, with continuous access style, only for 64*64 matrix";
void transpose_64_square_matrix_v2(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 64) {
        return;
    }

    const int BSIZE = 4;

    // block upper left index (i, j)
    size_t bul_i = 0, bul_j = 0;
    // the stride to move to next block
    size_t delta_i, delta_j;
    // inner-block index
    size_t i, j;

    while (bul_i < N && bul_j < N) {
        for (i = bul_i; i < bul_i + BSIZE; i++) {
            j = bul_j;
            trans_bsize_4(i, j, M, N, A, B);
        }

        if ((bul_j / 4) % 2 == 0) {
            if (bul_i == 0) {
                bul_j += BSIZE;
                delta_i = BSIZE;
                delta_j = -BSIZE;
                continue;
            } else if (bul_i == N - BSIZE) {
                bul_j += BSIZE;
                delta_i = -BSIZE;
                delta_j = BSIZE;
                continue;
            }
        }
        if ((bul_i / 4) % 2 == 1) {
            if (bul_j == 0) {
                bul_i += BSIZE;
                delta_i = -BSIZE;
                delta_j = BSIZE;
                continue;
            } else if (bul_j == N - BSIZE) {
                bul_i += BSIZE;
                delta_i = BSIZE;
                delta_j = -BSIZE;
                continue;
            }
        }

        bul_i += delta_i;
        bul_j += delta_j;
    }
}

char trans_64_square_matrix_v3_desc[] = "Block wise transpose, with inner block clock-wise access pattern, only for 64*64 matrix";
void transpose_64_square_matrix_v3(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 64) {
        return;
    }

    const int BSIZE = 8;
    const int INNER_BSIZE = 4;
    size_t bi, bj;
    size_t i, j;

    for (bi = 0; bi < N; bi += BSIZE) {
        for (bj = 0; bj < N; bj += BSIZE) {

            // inner upper left
            for (i = bi; i < bi + INNER_BSIZE; i++) {
                j = bj;
                trans_bsize_4(i, j, M, N, A, B);
            }

            // inner upper right
            for (i = bi; i < bi + INNER_BSIZE; i++) {
                j = bj + INNER_BSIZE;
                trans_bsize_4(i, j, M, N, A, B);
            }

            // inner bottom right
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i++) {
                j = bj + INNER_BSIZE;
                trans_bsize_4(i, j, M, N, A, B);
            }

            // inner bottom left
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i++) {
                j = bj;
                trans_bsize_4(i, j, M, N, A, B);
            }
        }
    }
}

char trans_64_square_matrix_v4_desc[] = "Block wise transpose, with inner block counter-clock-wise access pattern, only for 64*64 matrix";
void transpose_64_square_matrix_v4(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 64) {
        return;
    }

    const int BSIZE = 8;
    const int INNER_BSIZE = 4;
    size_t bi, bj;
    size_t i, j;

    for (bi = 0; bi < N; bi += BSIZE) {
        for (bj = 0; bj < N; bj += BSIZE) {

            // inner upper left
            for (i = bi; i < bi + INNER_BSIZE; i++) {
                j = bj;
                trans_bsize_4(i, j, M, N, A, B);
            }

            // inner bottom left
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i++) {
                j = bj;
                trans_bsize_4(i, j, M, N, A, B);
            }

            // inner bottom right
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i++) {
                j = bj + INNER_BSIZE;
                trans_bsize_4(i, j, M, N, A, B);
            }

            // inner upper right
            for (i = bi; i < bi + INNER_BSIZE; i++) {
                j = bj + INNER_BSIZE;
                trans_bsize_4(i, j, M, N, A, B);
            }
        }
    }
}

char trans_64_square_matrix_v5_desc[] = "Block wise transpose, with inner block clock-wise access pattern and 2*4 inner blocksize, only for 64*64 matrix";
void transpose_64_square_matrix_v5(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 64) {
        return;
    }

    const int BSIZE = 8;
    const int INNER_BSIZE = 4;
    size_t bi, bj;
    size_t i, j;

    for (bi = 0; bi < N; bi += BSIZE) {
        for (bj = 0; bj < N; bj += BSIZE) {

            // inner upper left
            for (i = bi; i < bi + INNER_BSIZE; i += 2) {
                j = bj;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // inner upper right
            for (i = bi; i < bi + INNER_BSIZE; i += 2) {
                j = bj + INNER_BSIZE;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // inner bottom right
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i += 2) {
                j = bj + INNER_BSIZE;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // inner bottom left
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i += 2) {
                j = bj;
                trans_bsize_2x4(i, j, M, N, A, B);
            }
        }
    }
}

char trans_64_square_matrix_v6_desc[] = "Block wise transpose, with inner block counter-clock-wise access pattern and 2*4 inner blocksize, only for 64*64 matrix";
void transpose_64_square_matrix_v6(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 64) {
        return;
    }

    const int BSIZE = 8;
    const int INNER_BSIZE = 4;
    size_t bi, bj;
    size_t i, j;

    for (bi = 0; bi < N; bi += BSIZE) {
        for (bj = 0; bj < N; bj += BSIZE) {

            // inner upper left
            for (i = bi; i < bi + INNER_BSIZE; i += 2) {
                j = bj;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // inner bottom left
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i += 2) {
                j = bj;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // inner bottom right
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i += 2) {
                j = bj + INNER_BSIZE;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // inner upper right
            for (i = bi; i < bi + INNER_BSIZE; i += 2) {
                j = bj + INNER_BSIZE;
                trans_bsize_2x4(i, j, M, N, A, B);
            }
        }
    }
}

char trans_64_square_matrix_v7_desc[] = "Block wise transpose, with inner block counter-clock-wise access pattern and 1*4 inner blocksize and temporarily store upper right 2*4 elements in local variables, only for 64*64 matrix";
void transpose_64_square_matrix_v7(int M, int N, int A[N][M], int B[M][N]) {
    if (M != N || N != 64) {
        return;
    }

    const int BSIZE = 8;
    const int INNER_BSIZE = 4;
    size_t bi, bj;
    size_t i, j;

    // temporary variables
    int a04, a05, a06, a07, a14, a15, a16, a17;

    for (bi = 0; bi < N; bi += BSIZE) {
        for (bj = 0; bj < N; bj += BSIZE) {

            // inner upper left
            trans_bsize_2x4(bi, bj, M, N, A, B);

            // store upper right 2*4 elements
            a04 = A[bi][bj + 4], a05 = A[bi][bj + 5], a06 = A[bi][bj + 6], a07 = A[bi][bj + 7],
            a14 = A[bi + 1][bj + 4], a15 = A[bi + 1][bj + 5], a16 = A[bi + 1][bj + 6], a17 = A[bi + 1][bj + 7];

            trans_bsize_2x4(bi + 2, bj, M, N, A, B);

            // inner bottom left
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i += 2) {
                j = bj;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // inner bottom right
            for (i = bi + INNER_BSIZE; i < bi + BSIZE; i += 2) {
                j = bj + INNER_BSIZE;
                trans_bsize_2x4(i, j, M, N, A, B);
            }

            // use stored 2*4 elements
            B[bj + 4][bi] = a04, B[bj + 5][bi] = a05, B[bj + 6][bi] = a06, B[bj + 7][bi] = a07,
                   B[bj + 4][bi + 1] = a14, B[bj + 5][bi + 1] = a15, B[bj + 6][bi + 1] = a16, B[bj + 7][bi + 1] = a17;

            // inner upper right
            trans_bsize_2x4(bi + 2, bj + INNER_BSIZE, M, N, A, B);
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
    if (M == 32 && N == 32) {
        transpose_32_square_matrix_v2(M, N, A, B);
    } else if (M == 64 && N == 64) {
        transpose_64_square_matrix_v7(M, N, A, B);
    } else {
        trans_row_wise(M, N, A, B);
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

    // registerTransFunction(transpose_64_square_matrix, trans_64_square_matrix_desc);
    // registerTransFunction(transpose_64_square_matrix_v2, trans_64_square_matrix_v2_desc);
    // registerTransFunction(transpose_64_square_matrix_v3, trans_64_square_matrix_v3_desc);
    // registerTransFunction(transpose_64_square_matrix_v4, trans_64_square_matrix_v4_desc);
    // registerTransFunction(transpose_64_square_matrix_v5, trans_64_square_matrix_v5_desc);
    // registerTransFunction(transpose_64_square_matrix_v6, trans_64_square_matrix_v6_desc);
    // registerTransFunction(transpose_64_square_matrix_v7, trans_64_square_matrix_v7_desc);
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
