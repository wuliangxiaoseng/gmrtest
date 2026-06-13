/**
 * @file runs.c
 * @brief 游程总数检测实现
 * 国密规范 GM/T 0005-2021
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include "tests.h"
#include "utils.h"

TestStatus test_runs(const BitData *data, int n, 
                     TestResult *result, const TestConfig *config)
{
    int i, V;
    double pi, p_value, q_value, numerator, denominator;
    double sqrt2 = 1.41421356237309504880;
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    /* 计算 π (1 的比例) */
    pi = 0.0;
    for (i = 0; i < n; i++) {
        pi += (double)GET_BIT(data, i);
    }
    pi /= (double)n;
    
    /* 如果 π 接近 0 或 1，序列不随机 */
    if (fabs(pi - 0.5) > 0.5 - 1e-10) {
        result->id = TEST_ID_RUNS_STR;
        result->name = "游程总数检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_FAILURE;
        result->message = "π 值接近 0 或 1";
        return TEST_FAILURE;
    }
    
    /* 计算游程总数 */
    V = 1;  /* 至少有一个游程 */
    for (i = 1; i < n; i++) {
        if (GET_BIT(data, i) != GET_BIT(data, i - 1)) {
            V++;
        }
    }
    
    /* 计算统计量 (国密规范公式) */
    numerator = fabs((double)V - 2.0 * (double)n * pi * (1.0 - pi));
    denominator = 2.0 * sqrt(2.0 * (double)n) * pi * (1.0 - pi);
    
    /* P_value = erfc(|V - 2nπ(1-π)| / (2√(2n)π(1-π))) */
    p_value = math_erfc(numerator / denominator);
    
    /* 国密规范: 正态分布型检测，Q_value 使用原始值（不取绝对值） */
    q_value = math_erfc((((double)V - 2.0 * (double)n * pi * (1.0 - pi))) / denominator);
    
    /* 设置结果 */
    result->id = TEST_ID_RUNS_STR;
    result->name = "游程总数检测";
    result->p_value = p_value;
    result->q_value = q_value;
    result->stat_value = (double)V;
    
    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}