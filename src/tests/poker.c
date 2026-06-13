/**
 * @file poker.c
 * @brief 扑克检测实现
 * 国密规范 GM/T 0005-2021 要求: m = 4, m = 8（两个独立检测项）
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "tests.h"
#include "utils.h"

static char poker_id_buf[2][32];
static char poker_name_buf[2][64];

/**
 * @brief 单参数扑克检测核心函数
 */
static TestStatus poker_single(const BitData *data, int n, int m, int idx,
                              TestResult *result, const TestConfig *config)
{
    int i, j, N, M;
    double V, p_value, q_value;
    uint32_t *count;
    int bit_pos;
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    M = 1 << m;  /* 2^m 种模式 */
    N = n / m;   /* 块数量 */
    
    if (N < 1 || m <= 0 || m > MAX_POKER_M) {
        return TEST_ERROR;
    }
    
    count = (uint32_t *)calloc(M, sizeof(uint32_t));
    if (count == NULL) {
        return TEST_ERROR;
    }
    
    /* 统计每种 m 位模式出现的频数 */
    for (i = 0; i < N; i++) {
        uint32_t pattern = 0;
        bit_pos = i * m;
        
        for (j = 0; j < m; j++) {
            pattern = (pattern << 1) | GET_BIT(data, bit_pos + j);
        }
        count[pattern]++;
    }
    
    /* 计算统计量 V */
    V = 0.0;
    for (i = 0; i < M; i++) {
        V += (double)count[i] * (double)count[i];
    }
    V = ((double)M / (double)N) * V - (double)N;
    
    /* 计算 p_value (卡方分布) */
    p_value = math_igamc((double)(M - 1) / 2.0, V / 2.0);
    
    /* 国密规范: 卡方分布型检测 Q_value = P_value */
    q_value = p_value;
    
    free(count);
    
    /* 设置结果 */
    snprintf(poker_id_buf[idx], sizeof(poker_id_buf[idx]), "poker_m%d", m);
    snprintf(poker_name_buf[idx], sizeof(poker_name_buf[idx]), "扑克检测 (m=%d)", m);
    
    result->id = poker_id_buf[idx];
    result->name = poker_name_buf[idx];
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

/**
 * @brief 扑克检测（国密规范要求 m=4, m=8）
 * 将结果存储到数组中
 */
int test_poker(const BitData *data, int n, TestResult *results, int max_results, 
               const TestConfig *config)
{
    int count = 0;
    
    if (results == NULL || max_results < 2) {
        return -1;
    }
    
    /* m=4 */
    if (n >= POKER_M_4 * 100) {  /* 至少需要 100 个块 */
        poker_single(data, n, POKER_M_4, 0, &results[count], config);
        count++;
    }
    
    /* m=8 */
    if (n >= POKER_M_8 * 100) {
        poker_single(data, n, POKER_M_8, 1, &results[count], config);
        count++;
    }
    
    return count;
}