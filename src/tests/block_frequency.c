/**
 * @file block_frequency.c
 * @brief 块内频数检测实现
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include "tests.h"
#include "utils.h"

TestStatus test_block_frequency(const BitData *data, int n, 
                                TestResult *result, const TestConfig *config)
{
    int i, j, N, M;
    double pi, sum, chi_squared, p_value, q_value;
    int block_start, block_end;
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    /* 国密规范 GM/T 0005-2021 要求 m=10000 */
    M = (config && config->block_length > 0) ? config->block_length : DEFAULT_BLOCK_FREQUENCY_M;
    
    if (M <= 0 || M > n) {
        return TEST_ERROR;
    }
    
    N = n / M;
    if (N < 1) {
        return TEST_ERROR;
    }
    
    sum = 0.0;
    
    for (i = 0; i < N; i++) {
        block_start = i * M;
        block_end = block_start + M;
        
        pi = 0.0;
        for (j = block_start; j < block_end; j++) {
            pi += (double)GET_BIT(data, j);
        }
        pi /= (double)M;
        
        sum += (pi - 0.5) * (pi - 0.5);
    }
    
    chi_squared = 4.0 * (double)M * sum;
    p_value = math_igamc((double)N / 2.0, chi_squared / 2.0);
    
    /* 国密规范 GM/T 0005-2021:
     * 卡方分布型检测: Q_value = P_value
     */
    q_value = p_value;
    
    result->id = TEST_ID_BLOCK_FREQUENCY_STR;
    result->name = "块内频数检测";
    result->p_value = p_value;
    result->q_value = q_value;
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