/**
 * @file universal.c
 * @brief Maurer 通用统计检测实现
 * 国密规范 GM/T 0005-2021: L=7, Q=1280
 * 参考: NIST SP 800-22 universal.c
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdlib.h>
#include "tests.h"
#include "utils.h"

/* L 从 6 到 16 的期望值与方差 (NIST 表格) */
static const double UNIVERSAL_EXPECTED[17] = {
    0, 0, 0, 0, 0, 0,
    5.2177052, 6.1962507, 7.1836656, 8.1764248, 9.1723243,
    10.170032, 11.168765, 12.168070, 13.167693, 14.167488, 15.167379
};
static const double UNIVERSAL_VARIANCE[17] = {
    0, 0, 0, 0, 0, 0,
    2.954, 3.125, 3.238, 3.311, 3.356, 3.384,
    3.401, 3.410, 3.416, 3.419, 3.421
};

TestStatus test_universal(const BitData *data, int n,
                          TestResult *result, const TestConfig *config)
{
    int i, j, p, L, Q, K;
    double arg, sqrt2, sigma, phi, sum, p_value, q_value, c;
    long *T = NULL, decRep;

    if (data == NULL || result == NULL || n <= 0 || config == NULL) {
        return TEST_ERROR;
    }

    /* 国密 L=7,Q=1280；若 n 不足以满足条件，自动按 NIST 选择 */
    L = UNIVERSAL_GM_L;
    Q = UNIVERSAL_GM_Q;
    /* 若数据非常大，可按 NIST 自适应；这里保持 L=7,Q=1280 */
    if (n < (Q + 100) * L) {
        result->id = TEST_ID_UNIVERSAL_STR;
        result->name = "Maurer通用统计检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足以执行 Maurer 检测";
        return TEST_ERROR;
    }
    K = (int)(floor((double)n / (double)L) - (double)Q);

    p = 1 << L;
    T = (long *)calloc(p, sizeof(long));
    if (T == NULL) {
        result->id = TEST_ID_UNIVERSAL_STR;
        result->name = "Maurer通用统计检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "内存分配失败";
        return TEST_ERROR;
    }

    c = 0.7 - 0.8 / (double)L + (4.0 + 32.0 / (double)L) * pow((double)K, -3.0 / (double)L) / 15.0;
    sigma = c * sqrt(UNIVERSAL_VARIANCE[L] / (double)K);
    sqrt2 = sqrt(2.0);
    sum = 0.0;

    for (i = 0; i < p; i++) T[i] = 0;
    for (i = 1; i <= Q; i++) {
        decRep = 0;
        for (j = 0; j < L; j++) {
            decRep += (long)GET_BIT(data, (i - 1) * L + j) * (1L << (L - 1 - j));
        }
        T[decRep] = i;
    }
    for (i = Q + 1; i <= Q + K; i++) {
        decRep = 0;
        for (j = 0; j < L; j++) {
            decRep += (long)GET_BIT(data, (i - 1) * L + j) * (1L << (L - 1 - j));
        }
        sum += log((double)(i - T[decRep])) / log(2.0);
        T[decRep] = i;
    }
    phi = sum / (double)K;

    arg = fabs(phi - UNIVERSAL_EXPECTED[L]) / (sqrt2 * sigma);
    p_value = math_erfc(arg);
    q_value = math_erfc((phi - UNIVERSAL_EXPECTED[L]) / (sqrt2 * sigma)) / 2.0;

    free(T);

    result->id = TEST_ID_UNIVERSAL_STR;
    result->name = "Maurer通用统计检测";
    result->p_value = p_value;
    result->q_value = q_value;
    result->stat_value = (phi - UNIVERSAL_EXPECTED[L]) / sigma;

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
