/**
 * @file matrix.h
 * @brief GF(2) 矩阵运算
 * @author gmrtest
 * @date 2024
 */

#ifndef GMRTEST_MATRIX_H
#define GMRTEST_MATRIX_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t **matrix_create(int M, int Q);
void matrix_free(uint8_t **mat, int M);
void matrix_define_from_bits(uint8_t **mat, int M, int Q, int k, const BitData *data);
int matrix_compute_rank_gf2(uint8_t **mat, int M, int Q);

#ifdef __cplusplus
}
#endif

#endif /* GMRTEST_MATRIX_H */
