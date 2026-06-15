/**
 * @file linear_complexity.c
 * @brief 线性复杂度检测实现
 * 国密规范 GM/T 0005-2021: m=500, m=1000
 * 参考: NIST SP 800-22 linearComplexity.c (Berlekamp-Massey)
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tests.h"
#include "utils.h"

static const char *id_for_lc(int M)
{
    if (M == LINEAR_COMPLEXITY_M_500) return TEST_ID_LINEAR_COMPLEXITY_M500_STR;
    if (M == LINEAR_COMPLEXITY_M_1000) return TEST_ID_LINEAR_COMPLEXITY_M1000_STR;
    return TEST_ID_LINEAR_COMPLEXITY_STR;
}

static const char *name_for_lc(int M)
{
    static char buf_500[64];
    static char buf_1000[64];
    static char buf_other[64];
    if (M == LINEAR_COMPLEXITY_M_500) {
        snprintf(buf_500, sizeof(buf_500), "线性复杂度检测 (m=%d)", M);
        return buf_500;
    }
    if (M == LINEAR_COMPLEXITY_M_1000) {
        snprintf(buf_1000, sizeof(buf_1000), "线性复杂度检测 (m=%d)", M);
        return buf_1000;
    }
    snprintf(buf_other, sizeof(buf_other), "线性复杂度检测 (m=%d)", M);
    return buf_other;
}

TestStatus test_linear_complexity(const BitData *data, int n,
                                  TestResult *result, const TestConfig *config)
{
    int i, ii, j, d, N, L, m, N_, parity, sign;
    int K = 6;
    int M;
    double T_, mean, p_value, chi2;
    double nu[7];
    static const double pi_p[7] = {
        0.010417, 0.03125, 0.12500, 0.50000,
        0.25000, 0.06250, 0.020833
    };
    uint8_t *T = NULL, *P = NULL, *B_ = NULL, *C = NULL;
    uint8_t *block = NULL;

    if (data == NULL || result == NULL || n <= 0 || config == NULL) {
        return TEST_ERROR;
    }
    M = config->block_length > 0 ? config->block_length : LINEAR_COMPLEXITY_M_500;
    N = (int)floor((double)n / (double)M);
    if (N == 0) {
        result->id = id_for_lc(M);
        result->name = name_for_lc(M);
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足以构成块";
        return TEST_ERROR;
    }

    B_ = (uint8_t *)calloc(M, sizeof(uint8_t));
    C  = (uint8_t *)calloc(M, sizeof(uint8_t));
    P  = (uint8_t *)calloc(M, sizeof(uint8_t));
    T  = (uint8_t *)calloc(M, sizeof(uint8_t));
    block = (uint8_t *)calloc(M, sizeof(uint8_t));
    if (!B_ || !C || !P || !T || !block) {
        if (B_) free(B_);
        if (C)  free(C);
        if (P)  free(P);
        if (T)  free(T);
        if (block) free(block);
        result->id = id_for_lc(M);
        result->name = name_for_lc(M);
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "内存分配失败";
        return TEST_ERROR;
    }

    for (i = 0; i < K + 1; i++) nu[i] = 0.0;

    for (ii = 0; ii < N; ii++) {
        /* 提取一个块 */
        for (i = 0; i < M; i++) {
            block[i] = (uint8_t)GET_BIT(data, ii * M + i);
            B_[i] = 0;
            C[i] = 0;
            T[i] = 0;
            P[i] = 0;
        }
        L = 0;
        m = -1;
        d = 0;
        C[0] = 1;
        B_[0] = 1;
        N_ = 0;
        while (N_ < M) {
            d = (int)block[N_];
            for (i = 1; i <= L; i++) {
                d += C[i] * block[N_ - i];
            }
            d = d % 2;
            if (d == 1) {
                for (i = 0; i < M; i++) {
                    T[i] = C[i];
                    P[i] = 0;
                }
                for (j = 0; j < M; j++) {
                    if (B_[j] == 1) {
                        if (j + N_ - m < M) {
                            P[j + N_ - m] = 1;
                        }
                    }
                }
                for (i = 0; i < M; i++) {
                    C[i] = (C[i] + P[i]) % 2;
                }
                if (L <= N_ / 2) {
                    L = N_ + 1 - L;
                    m = N_;
                    for (i = 0; i < M; i++) {
                        B_[i] = T[i];
                    }
                }
            }
            N_++;
        }
        if ((parity = (M + 1) % 2) == 0) sign = -1;
        else sign = 1;
        mean = M / 2.0 + (9.0 + sign) / 36.0 - 1.0 / pow(2, M) * (M / 3.0 + 2.0 / 9.0);
        if ((parity = M % 2) == 0) sign = 1;
        else sign = -1;
        T_ = sign * (L - mean) + 2.0 / 9.0;

        if (T_ <= -2.5) nu[0]++;
        else if (T_ <= -1.5) nu[1]++;
        else if (T_ <= -0.5) nu[2]++;
        else if (T_ <=  0.5) nu[3]++;
        else if (T_ <=  1.5) nu[4]++;
        else if (T_ <=  2.5) nu[5]++;
        else nu[6]++;
    }

    chi2 = 0.0;
    for (i = 0; i <= K; i++) {
        chi2 += pow(nu[i] - (double)N * pi_p[i], 2) / ((double)N * pi_p[i]);
    }
    p_value = math_igamc((double)K / 2.0, chi2 / 2.0);

    free(B_);
    free(C);
    free(P);
    free(T);
    free(block);

    result->id = id_for_lc(M);
    result->name = name_for_lc(M);
    result->p_value = p_value;
    result->q_value = p_value;
    result->stat_value = chi2;

    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}

TestStatus test_linear_complexity_m500(const BitData *data, int n,
                                       TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.block_length = LINEAR_COMPLEXITY_M_500;
    return test_linear_complexity(data, n, result, &cfg);
}

TestStatus test_linear_complexity_m1000(const BitData *data, int n,
                                        TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.block_length = LINEAR_COMPLEXITY_M_1000;
    return test_linear_complexity(data, n, result, &cfg);
}
