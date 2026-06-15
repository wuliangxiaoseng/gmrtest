/**
 * @file matrix.c
 * @brief GF(2) 二元矩阵秩计算
 * 参考: NIST/RandomTest matrix.c (computeRank)
 * @author gmrtest
 * @date 2024
 */

#include <stdlib.h>
#include "utils.h"
#include "matrix.h"

static int matrix_swap_rows(int i, int idx, int Q, uint8_t **A)
{
    int p;
    uint8_t tmp;
    for (p = 0; p < Q; p++) {
        tmp = A[i][p];
        A[i][p] = A[idx][p];
        A[idx][p] = tmp;
    }
    return 1;
}

static int matrix_find_unit_and_swap(int forward, int i, int M, int Q, uint8_t **A)
{
    int idx;
    if (forward) {
        idx = i + 1;
        while (idx < M && A[idx][i] == 0) idx++;
        if (idx < M) return matrix_swap_rows(i, idx, Q, A);
    } else {
        idx = i - 1;
        while (idx >= 0 && A[idx][i] == 0) idx--;
        if (idx >= 0) return matrix_swap_rows(i, idx, Q, A);
    }
    return 0;
}

static void matrix_row_op(int forward, int i, int M, int Q, uint8_t **A)
{
    int j, k;
    if (forward) {
        for (j = i + 1; j < M; j++) {
            if (A[j][i] == 1) {
                for (k = i; k < Q; k++) {
                    A[j][k] = (A[j][k] + A[i][k]) % 2;
                }
            }
        }
    } else {
        for (j = i - 1; j >= 0; j--) {
            if (A[j][i] == 1) {
                for (k = 0; k < Q; k++) {
                    A[j][k] = (A[j][k] + A[i][k]) % 2;
                }
            }
        }
    }
}

uint8_t **matrix_create(int M, int Q)
{
    int i;
    uint8_t **mat = (uint8_t **)calloc(M, sizeof(uint8_t *));
    if (mat == NULL) return NULL;
    for (i = 0; i < M; i++) {
        mat[i] = (uint8_t *)calloc(Q, sizeof(uint8_t));
        if (mat[i] == NULL) {
            int k;
            for (k = 0; k < i; k++) free(mat[k]);
            free(mat);
            return NULL;
        }
    }
    return mat;
}

void matrix_free(uint8_t **mat, int M)
{
    int i;
    if (mat == NULL) return;
    for (i = 0; i < M; i++) {
        if (mat[i]) free(mat[i]);
    }
    free(mat);
}

void matrix_define_from_bits(uint8_t **mat, int M, int Q, int k, const BitData *data)
{
    int i, j;
    for (i = 0; i < M; i++) {
        for (j = 0; j < Q; j++) {
            mat[i][j] = (uint8_t)GET_BIT(data, k * (M * Q) + j + i * Q);
        }
    }
}

int matrix_compute_rank_gf2(uint8_t **mat, int M, int Q)
{
    int i, j, rank;
    int m = (M < Q) ? M : Q;

    /* forward elimination */
    for (i = 0; i < m - 1; i++) {
        if (mat[i][i] == 1) {
            matrix_row_op(1, i, M, Q, mat);
        } else {
            if (matrix_find_unit_and_swap(1, i, M, Q, mat) == 1) {
                matrix_row_op(1, i, M, Q, mat);
            }
        }
    }
    /* backward elimination */
    for (i = m - 1; i > 0; i--) {
        if (mat[i][i] == 1) {
            matrix_row_op(0, i, M, Q, mat);
        } else {
            if (matrix_find_unit_and_swap(0, i, M, Q, mat) == 1) {
                matrix_row_op(0, i, M, Q, mat);
            }
        }
    }
    /* count nonzero rows */
    rank = m;
    for (i = 0; i < M; i++) {
        int allzero = 1;
        for (j = 0; j < Q; j++) {
            if (mat[i][j] == 1) {
                allzero = 0;
                break;
            }
        }
        if (allzero) rank--;
    }
    return rank;
}
