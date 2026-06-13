/**
 * @file runs_distribution.c
 * @brief 游程分布检测实现
 * 国密规范 GM/T 0005-2021
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdlib.h>
#include "tests.h"
#include "utils.h"

TestStatus test_runs_distribution(const BitData *data, int n, 
                                  TestResult *result, const TestConfig *config)
{
    int i, j, k;
    double V = 0.0, p_value = 0.0, q_value = 0.0;
    double *bi = NULL, *gi = NULL, *e = NULL;
    double bit, git, et;
    int current_bit, current_length;
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    bi = (double *)calloc(n, sizeof(double));
    if (bi == NULL) {
        return TEST_ERROR;
    }
    gi = (double *)calloc(n, sizeof(double));
    if (gi == NULL) {
        free(bi);
        return TEST_ERROR;
    }
    e = (double *)calloc(n, sizeof(double));
    if (e == NULL) {
        free(bi);
        free(gi);
        return TEST_ERROR;
    }
    
    /* 计算期望频数 e[i] = (n - i + 3) / 2^(i + 2) */
    k = 0;
    for (i = 1; i <= n; ++i) {
        e[i-1] = (double)(n - i + 3) / pow(2.0, (double)(i + 2));
        if (e[i-1] >= 5.0) {
            k = i;
        }
    }
    
    /* 如果没有满足条件的游程长度 */
    if (k == 0) {
        free(bi);
        free(gi);
        free(e);
        result->id = TEST_ID_RUNS_DISTRIBUTION_STR;
        result->name = "游程分布检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_FAILURE;
        result->message = "无法计算有效游程长度";
        return TEST_FAILURE;
    }
    
    /* 统计游程长度分布 */
    current_bit = GET_BIT(data, 0);
    current_length = 1;
    
    for (i = 1; i < n; ++i) {
        if (GET_BIT(data, i) != current_bit) {
            if (current_bit == 0) {
                if (current_length - 1 < n) {
                    gi[current_length - 1] += 1.0;
                }
            } else {
                if (current_length - 1 < n) {
                    bi[current_length - 1] += 1.0;
                }
            }
            current_bit = GET_BIT(data, i);
            current_length = 1;
        } else {
            ++current_length;
        }
    }
    
    /* 记录最后一个游程 */
    if (current_bit == 0) {
        if (current_length - 1 < n) {
            gi[current_length - 1] += 1.0;
        }
    } else {
        if (current_length - 1 < n) {
            bi[current_length - 1] += 1.0;
        }
    }
    
    /* 计算统计量 V */
    double sum_bi = 0.0, sum_gi = 0.0;
    for (i = 0; i < k; ++i) {
        bit = bi[i];
        et = e[i];
        sum_bi += pow(bit - et, 2.0) / et;
    }
    for (i = 0; i < k; ++i) {
        git = gi[i];
        et = e[i];
        sum_gi += pow(git - et, 2.0) / et;
    }
    V = sum_bi + sum_gi;
    
    /* 计算 p 值，自由度 = k - 1 */
    p_value = math_igamc((double)(k - 1), V / 2.0);
    
    /* 国密规范: 卡方分布型检测 Q_value = P_value */
    q_value = p_value;
    
    free(bi);
    free(gi);
    free(e);
    
    /* 设置结果 */
    result->id = TEST_ID_RUNS_DISTRIBUTION_STR;
    result->name = "游程分布检测";
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