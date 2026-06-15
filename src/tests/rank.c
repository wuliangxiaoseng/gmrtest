/**
 * @file rank.c
 * @brief 矩阵秩检测实现 (32x32 二元矩阵)
 * 国密规范 GM/T 0005-2021
 * 参考: NIST SP 800-22 rank.c
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdlib.h>
#include "tests.h"
#include "utils.h"
#include "matrix.h"

#define RANK_M 32
#define RANK_Q 32

TestStatus test_rank(const BitData *data, int n,
                     TestResult *result, const TestConfig *config)
{
    int N, k, R, i;
    double p_value, product, chi_squared, p_32, p_31, p_30, F_32, F_31, F_30;
    uint8_t **mat = NULL;

    if (data == NULL || result == NULL || n <= 0 || config == NULL) {
        return TEST_ERROR;
    }

    N = n / (RANK_M * RANK_Q);
    if (N == 0) {
        result->id = TEST_ID_RANK_STR;
        result->name = "矩阵秩检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足以构成32x32矩阵";
        return TEST_ERROR;
    }

    mat = matrix_create(RANK_M, RANK_Q);
    if (mat == NULL) {
        result->id = TEST_ID_RANK_STR;
        result->name = "矩阵秩检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "内存分配失败";
        return TEST_ERROR;
    }

    /* 概率值 */
    {
        int r;
        r = 32;
        product = 1.0;
        for (i = 0; i <= r - 1; i++) {
            product *= ((1.0 - pow(2, i - 32)) * (1.0 - pow(2, i - 32))) / (1.0 - pow(2, i - r));
        }
        p_32 = pow(2, r * (32 + 32 - r) - 32 * 32) * product;

        r = 31;
        product = 1.0;
        for (i = 0; i <= r - 1; i++) {
            product *= ((1.0 - pow(2, i - 32)) * (1.0 - pow(2, i - 32))) / (1.0 - pow(2, i - r));
        }
        p_31 = pow(2, r * (32 + 32 - r) - 32 * 32) * product;
        p_30 = 1.0 - (p_32 + p_31);
    }

    F_32 = 0;
    F_31 = 0;
    for (k = 0; k < N; k++) {
        matrix_define_from_bits(mat, RANK_M, RANK_Q, k, data);
        R = matrix_compute_rank_gf2(mat, RANK_M, RANK_Q);
        if (R == 32) F_32++;
        else if (R == 31) F_31++;
    }
    F_30 = (double)N - (F_32 + F_31);

    chi_squared = (pow(F_32 - N * p_32, 2) / (N * p_32) +
                   pow(F_31 - N * p_31, 2) / (N * p_31) +
                   pow(F_30 - N * p_30, 2) / (N * p_30));
    p_value = exp(-chi_squared / 2.0);

    matrix_free(mat, RANK_M);

    result->id = TEST_ID_RANK_STR;
    result->name = "矩阵秩检测";
    result->p_value = p_value;
    result->q_value = p_value;     /* 卡方型 */
    result->stat_value = chi_squared;

    if (p_value < 0.0 || p_value > 1.0) {
        result->status = TEST_ERROR;
        result->message = "p值越界";
        return TEST_ERROR;
    }
    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}
