/**
 * @file self_correlation.c
 * @brief 自相关检测实现
 * 国密规范 GM/T 0005-2021: d=1, 2, 8, 16
 * 参考: NIST/RandomTest selfCorrelation.c
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdio.h>
#include "tests.h"
#include "utils.h"

static const char *id_for_d(int d)
{
    switch (d) {
        case 1:  return TEST_ID_AUTOCORRELATION_D1_STR;
        case 2:  return TEST_ID_AUTOCORRELATION_D2_STR;
        case 8:  return TEST_ID_AUTOCORRELATION_D8_STR;
        case 16: return TEST_ID_AUTOCORRELATION_D16_STR;
        default: return TEST_ID_AUTOCORRELATION_STR;
    }
}

static const char *name_for_d(int d)
{
    static char buf_d1[64];
    static char buf_d2[64];
    static char buf_d8[64];
    static char buf_d16[64];
    static char buf_other[64];
    switch (d) {
        case 1:
            snprintf(buf_d1, sizeof(buf_d1), "自相关检测 (d=%d)", d);
            return buf_d1;
        case 2:
            snprintf(buf_d2, sizeof(buf_d2), "自相关检测 (d=%d)", d);
            return buf_d2;
        case 8:
            snprintf(buf_d8, sizeof(buf_d8), "自相关检测 (d=%d)", d);
            return buf_d8;
        case 16:
            snprintf(buf_d16, sizeof(buf_d16), "自相关检测 (d=%d)", d);
            return buf_d16;
        default:
            snprintf(buf_other, sizeof(buf_other), "自相关检测 (d=%d)", d);
            return buf_other;
    }
}

TestStatus test_self_correlation(const BitData *data, int n,
                                 TestResult *result, const TestConfig *config)
{
    int i, d, n_d, Ad;
    double V, p_value, q_value, sqrt2 = 1.41421356237309504880;

    if (data == NULL || result == NULL || n <= 0 || config == NULL) {
        return TEST_ERROR;
    }

    d = config->correlation_delay > 0 ? config->correlation_delay : 1;
    n_d = n - d;
    if (n_d <= 0) {
        result->id = id_for_d(d);
        result->name = name_for_d(d);
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足";
        return TEST_ERROR;
    }

    Ad = 0;
    for (i = 0; i < n_d; i++) {
        Ad += (GET_BIT(data, i) ^ GET_BIT(data, i + d));
    }

    V = 2.0 * ((double)Ad - ((double)n_d / 2.0)) / sqrt((double)n_d);

    p_value = math_erfc(fabs(V) / sqrt2);
    q_value = math_erfc(V / sqrt2) / 2.0;

    result->id = id_for_d(d);
    result->name = name_for_d(d);
    result->p_value = p_value;
    result->q_value = q_value;
    result->stat_value = V;

    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}

TestStatus test_self_correlation_d1(const BitData *data, int n,
                                    TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.correlation_delay = AUTOCORRELATION_D_1;
    return test_self_correlation(data, n, result, &cfg);
}

TestStatus test_self_correlation_d2(const BitData *data, int n,
                                    TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.correlation_delay = AUTOCORRELATION_D_2;
    return test_self_correlation(data, n, result, &cfg);
}

TestStatus test_self_correlation_d8(const BitData *data, int n,
                                    TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.correlation_delay = AUTOCORRELATION_D_8;
    return test_self_correlation(data, n, result, &cfg);
}

TestStatus test_self_correlation_d16(const BitData *data, int n,
                                     TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.correlation_delay = AUTOCORRELATION_D_16;
    return test_self_correlation(data, n, result, &cfg);
}
