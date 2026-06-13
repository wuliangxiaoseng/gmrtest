/**
 * @file serial.c
 * @brief 重叠子序列检测实现
 * 国密规范 GM/T 0005-2021 要求: m=3, m=5（各返回2个p值）
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "tests.h"
#include "utils.h"

static char serial_id_buf[4][32];
static char serial_name_buf[4][64];

/**
 * @brief 单参数重叠子序列检测核心函数
 * 返回两个 p_value
 */
static int serial_single(const BitData *data, int n, int m, int base_idx,
                        TestResult *result1, TestResult *result2,
                        const TestConfig *config)
{
    int i, j, N, M;
    double *C, *C1, psi_sq_m, psi_sq_m1, delta1, delta2;
    double p_value1, p_value2, q_value1, q_value2;
    int bit_pos;
    
    if (data == NULL || result1 == NULL || result2 == NULL || n <= 0) {
        return -1;
    }
    
    M = 1 << m;      /* 2^m */
    N = n - m + 1;   /* 重叠块数量 */
    
    if (N < M || m <= 0) {
        return -1;
    }
    
    C = (double *)calloc(M, sizeof(double));
    C1 = (double *)calloc(M, sizeof(double));
    if (C == NULL || C1 == NULL) {
        free(C);
        free(C1);
        return -1;
    }
    
    /* 统计 m 位模式和 m-1 位模式的频数 */
    for (i = 0; i < N; i++) {
        uint32_t pattern_m = 0;
        uint32_t pattern_m1 = 0;
        bit_pos = i;
        
        for (j = 0; j < m; j++) {
            pattern_m = (pattern_m << 1) | GET_BIT(data, bit_pos + j);
        }
        pattern_m1 = pattern_m >> 1;
        
        C[pattern_m]++;
        
        if (i < N - 1) {
            C1[pattern_m1]++;
        }
    }
    
    /* 计算 ψ²(m) */
    psi_sq_m = 0.0;
    for (i = 0; i < M; i++) {
        psi_sq_m += C[i] * C[i];
    }
    psi_sq_m = (M * psi_sq_m) / (double)N - (double)N;
    
    /* 计算 ψ²(m+1) */
    psi_sq_m1 = 0.0;
    for (i = 0; i < (M >> 1); i++) {
        psi_sq_m1 += C1[i] * C1[i];
    }
    psi_sq_m1 = ((M >> 1) * psi_sq_m1) / (double)(N - 1) - (double)(N - 1);
    
    /* 计算 Δψ² */
    delta1 = psi_sq_m - psi_sq_m1;
    delta2 = psi_sq_m1;
    
    /* 确保 delta 为非负数 */
    if (delta1 < 0.0) delta1 = 0.0;
    if (delta2 < 0.0) delta2 = 0.0;
    
    /* 计算 p_values */
    p_value1 = math_igamc((double)(M >> 1) / 2.0, delta1 / 2.0);
    p_value2 = math_igamc((double)(M >> 1) / 2.0, delta2 / 2.0);
    
    /* 国密规范: 卡方分布型检测 Q_value = P_value */
    q_value1 = p_value1;
    q_value2 = p_value2;
    
    free(C);
    free(C1);
    
    /* 设置结果 */
    snprintf(serial_id_buf[base_idx], sizeof(serial_id_buf[base_idx]), "serial_m%d_p1", m);
    snprintf(serial_name_buf[base_idx], sizeof(serial_name_buf[base_idx]), "重叠子序列检测 (m=%d, P1)", m);
    snprintf(serial_id_buf[base_idx+1], sizeof(serial_id_buf[base_idx+1]), "serial_m%d_p2", m);
    snprintf(serial_name_buf[base_idx+1], sizeof(serial_name_buf[base_idx+1]), "重叠子序列检测 (m=%d, P2)", m);
    
    result1->id = serial_id_buf[base_idx];
    result1->name = serial_name_buf[base_idx];
    result1->p_value = p_value1;
    result1->q_value = q_value1;
    result1->stat_value = delta1;
    result1->status = (p_value1 < config->alpha) ? TEST_FAILURE : TEST_SUCCESS;
    result1->message = (p_value1 < config->alpha) ? "p值小于显著性水平" : "通过";
    
    result2->id = serial_id_buf[base_idx+1];
    result2->name = serial_name_buf[base_idx+1];
    result2->p_value = p_value2;
    result2->q_value = q_value2;
    result2->stat_value = delta2;
    result2->status = (p_value2 < config->alpha) ? TEST_FAILURE : TEST_SUCCESS;
    result2->message = (p_value2 < config->alpha) ? "p值小于显著性水平" : "通过";
    
    return 0;
}

/**
 * @brief 重叠子序列检测（国密规范要求 m=3, m=5，各返回2个p值）
 * 返回结果数量
 */
int test_serial(const BitData *data, int n, TestResult *results, int max_results,
               const TestConfig *config)
{
    int count = 0;
    
    if (results == NULL || max_results < 4) {
        return -1;
    }
    
    /* m=3 */
    if (n >= SERIAL_M_3 + 100) {
        serial_single(data, n, SERIAL_M_3, 0, &results[count], &results[count + 1], config);
        count += 2;
    }
    
    /* m=5 */
    if (n >= SERIAL_M_5 + 100) {
        serial_single(data, n, SERIAL_M_5, 2, &results[count], &results[count + 1], config);
        count += 2;
    }
    
    return count;
}