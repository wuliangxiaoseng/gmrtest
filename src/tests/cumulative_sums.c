/**
 * @file cumulative_sums.c
 * @brief 累加和检测实现
 * 国密规范 GM/T 0005-2021: forward / backward 两个独立检测项
 * 参考: NIST SP 800-22 cusum.c
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdlib.h>
#include "tests.h"
#include "utils.h"

/**
 * @brief 累加和检测核心
 * 通过 config->cusum_mode 选择前向(0)或后向(1)
 */
TestStatus test_cumulative_sums(const BitData *data, int n,
                                TestResult *result, const TestConfig *config)
{
    int S, sup, inf, z, zrev, k;
    double sum1, sum2, p_value, q_value;
    int z_use;
    int mode;

    if (data == NULL || result == NULL || n <= 0 || config == NULL) {
        return TEST_ERROR;
    }

    mode = config->cusum_mode;

    /* 计算 S, sup, inf, 以及前向 z 与后向 zrev */
    S = 0;
    sup = 0;
    inf = 0;
    for (k = 0; k < n; k++) {
        S += GET_BIT(data, k) ? 1 : -1;
        if (S > sup) sup++;
        if (S < inf) inf--;
    }
    z = (sup > -inf) ? sup : -inf;
    zrev = ((sup - S) > (S - inf)) ? (sup - S) : (S - inf);

    z_use = (mode == 0) ? z : zrev;

    if (z_use == 0) {
        result->id = (mode == 0) ? TEST_ID_CUMULATIVE_SUMS_FORWARD_STR
                                 : TEST_ID_CUMULATIVE_SUMS_BACKWARD_STR;
        result->name = (mode == 0) ? "累加和检测(前向)" : "累加和检测(后向)";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "z=0，无法计算";
        return TEST_ERROR;
    }

    sum1 = 0.0;
    for (k = (-n / z_use + 1) / 4; k <= (n / z_use - 1) / 4; k++) {
        sum1 += math_normal(((4 * k + 1) * z_use) / sqrt((double)n));
        sum1 -= math_normal(((4 * k - 1) * z_use) / sqrt((double)n));
    }
    sum2 = 0.0;
    for (k = (-n / z_use - 3) / 4; k <= (n / z_use - 1) / 4; k++) {
        sum2 += math_normal(((4 * k + 3) * z_use) / sqrt((double)n));
        sum2 -= math_normal(((4 * k + 1) * z_use) / sqrt((double)n));
    }

    p_value = 1.0 - sum1 + sum2;
    q_value = p_value;

    if (p_value < 0.0) p_value = 0.0;
    if (p_value > 1.0) p_value = 1.0;

    result->id = (mode == 0) ? TEST_ID_CUMULATIVE_SUMS_FORWARD_STR
                             : TEST_ID_CUMULATIVE_SUMS_BACKWARD_STR;
    result->name = (mode == 0) ? "累加和检测(前向)" : "累加和检测(后向)";
    result->p_value = p_value;
    result->q_value = q_value;
    result->stat_value = (double)z_use;

    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}

TestStatus test_cumulative_sums_forward(const BitData *data, int n,
                                        TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.cusum_mode = 0;
    return test_cumulative_sums(data, n, result, &cfg);
}

TestStatus test_cumulative_sums_backward(const BitData *data, int n,
                                         TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.cusum_mode = 1;
    return test_cumulative_sums(data, n, result, &cfg);
}
