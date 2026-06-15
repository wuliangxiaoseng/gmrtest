/**
 * @file approximate_entropy.c
 * @brief 近似熵检测实现
 * 国密规范 GM/T 0005-2021: m=2, m=5
 * 参考: NIST SP 800-22 approximateEntropy.c
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tests.h"
#include "utils.h"

static const char *id_for_m(int m)
{
    if (m == APPROX_ENTROPY_M_2) return TEST_ID_APPROXIMATE_ENTROPY_M2_STR;
    if (m == APPROX_ENTROPY_M_5) return TEST_ID_APPROXIMATE_ENTROPY_M5_STR;
    return TEST_ID_APPROXIMATE_ENTROPY_STR;
}

static const char *name_for_m(int m)
{
    static char buf_m2[64];
    static char buf_m5[64];
    static char buf_other[64];
    if (m == APPROX_ENTROPY_M_2) {
        snprintf(buf_m2, sizeof(buf_m2), "近似熵检测 (m=%d)", m);
        return buf_m2;
    }
    if (m == APPROX_ENTROPY_M_5) {
        snprintf(buf_m5, sizeof(buf_m5), "近似熵检测 (m=%d)", m);
        return buf_m5;
    }
    snprintf(buf_other, sizeof(buf_other), "近似熵检测 (m=%d)", m);
    return buf_other;
}

TestStatus test_approximate_entropy(const BitData *data, int n,
                                    TestResult *result, const TestConfig *config)
{
    int i, j, k, r, blockSize, powLen, index;
    double sum, numOfBlocks, ApEn[2], apen, chi_squared, p_value;
    unsigned int *P = NULL;
    int m;

    if (data == NULL || result == NULL || n <= 0 || config == NULL) {
        return TEST_ERROR;
    }
    m = config->block_length > 0 ? config->block_length : APPROX_ENTROPY_M_2;

    r = 0;
    ApEn[0] = 0.0;
    ApEn[1] = 0.0;

    for (blockSize = m; blockSize <= m + 1; blockSize++) {
        if (blockSize == 0) {
            ApEn[0] = 0.0;
            r++;
            continue;
        }
        numOfBlocks = (double)n;
        powLen = (int)pow(2, blockSize + 1) - 1;
        P = (unsigned int *)calloc(powLen, sizeof(unsigned int));
        if (P == NULL) {
            result->id = id_for_m(m);
            result->name = name_for_m(m);
            result->p_value = 0.0;
            result->q_value = 0.0;
            result->stat_value = 0.0;
            result->status = TEST_ERROR;
            result->message = "内存分配失败";
            return TEST_ERROR;
        }
        for (i = 0; i < n; i++) {
            k = 1;
            for (j = 0; j < blockSize; j++) {
                k <<= 1;
                if ((int)GET_BIT(data, (i + j) % n) == 1) {
                    k++;
                }
            }
            P[k - 1]++;
        }
        sum = 0.0;
        index = (int)pow(2, blockSize) - 1;
        for (i = 0; i < (int)pow(2, blockSize); i++) {
            if (P[index] > 0) {
                sum += P[index] * log(P[index] / numOfBlocks);
            }
            index++;
        }
        sum /= numOfBlocks;
        ApEn[r] = sum;
        r++;
        free(P);
        P = NULL;
    }
    apen = ApEn[0] - ApEn[1];
    chi_squared = 2.0 * (double)n * (log(2.0) - apen);
    p_value = math_igamc(pow(2, m - 1), chi_squared / 2.0);

    result->id = id_for_m(m);
    result->name = name_for_m(m);
    result->p_value = p_value;
    result->q_value = p_value;     /* 卡方型：Q=P */
    result->stat_value = chi_squared;

    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}

TestStatus test_approximate_entropy_m2(const BitData *data, int n,
                                       TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.block_length = APPROX_ENTROPY_M_2;
    return test_approximate_entropy(data, n, result, &cfg);
}

TestStatus test_approximate_entropy_m5(const BitData *data, int n,
                                       TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.block_length = APPROX_ENTROPY_M_5;
    return test_approximate_entropy(data, n, result, &cfg);
}
