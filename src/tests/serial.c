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
    int i, j, M, M1, M2;
    double *C, *C1, *C2, psi_sq_m, psi_sq_m1, psi_sq_m2, delta1, delta2;
    double p_value1, p_value2, q_value1, q_value2;
    
    if (data == NULL || result1 == NULL || result2 == NULL || n <= 0) {
        return -1;
    }
    
    M = 1 << m;        /* 2^m */
    M1 = 1 << (m - 1); /* 2^(m-1) */
    M2 = 1 << (m - 2); /* 2^(m-2) */
    
    if (n < M || m < 2) {
        return -1;
    }
    
    C  = (double *)calloc(M,  sizeof(double));
    C1 = (double *)calloc(M1, sizeof(double));
    C2 = (double *)calloc(M2, sizeof(double));
    if (C == NULL || C1 == NULL || C2 == NULL) {
        free(C);
        free(C1);
        free(C2);
        return -1;
    }
    
    /* 统计 m / m-1 / m-2 位重叠模式频数（环形拼接，共 n 个重叠子序列） */
    for (i = 0; i < n; i++) {
        uint32_t pattern_m = 0;
        
        for (j = 0; j < m; j++) {
            pattern_m = (pattern_m << 1) | GET_BIT(data, (i + j) % n);
        }
        
        C[pattern_m]++;
        C1[pattern_m >> 1]++;
        C2[pattern_m >> 2]++;
    }
    
    /* 计算 ψ²(m)、ψ²(m-1)、ψ²(m-2)，均除以 n */
    psi_sq_m = 0.0;
    for (i = 0; i < M; i++) {
        psi_sq_m += C[i] * C[i];
    }
    psi_sq_m = ((double)M / n) * psi_sq_m - (double)n;
    
    psi_sq_m1 = 0.0;
    for (i = 0; i < M1; i++) {
        psi_sq_m1 += C1[i] * C1[i];
    }
    psi_sq_m1 = ((double)M1 / n) * psi_sq_m1 - (double)n;
    
    psi_sq_m2 = 0.0;
    for (i = 0; i < M2; i++) {
        psi_sq_m2 += C2[i] * C2[i];
    }
    psi_sq_m2 = ((double)M2 / n) * psi_sq_m2 - (double)n;
    
    /* 计算 ∇ψ²(m) 和 ∇²ψ²(m) */
    delta1 = psi_sq_m - psi_sq_m1;
    delta2 = psi_sq_m - 2.0 * psi_sq_m1 + psi_sq_m2;
    
    /* 确保 delta 为非负数 */
    if (delta1 < 0.0) delta1 = 0.0;
    if (delta2 < 0.0) delta2 = 0.0;
    
    /* 计算 p_values：P1=igamc(2^(m-2), ∇ψ²/2)，P2=igamc(2^(m-3), ∇²ψ²/2) */
    p_value1 = math_igamc((double)M / 4.0, delta1 / 2.0);
    p_value2 = math_igamc((double)M / 8.0, delta2 / 2.0);
    
    /* 国密规范: 卡方分布型检测 Q_value = P_value */
    q_value1 = p_value1;
    q_value2 = p_value2;
    
    free(C);
    free(C1);
    free(C2);
    
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