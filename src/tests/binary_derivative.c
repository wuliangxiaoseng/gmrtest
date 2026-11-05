/**
 * @file binary_derivative.c
 * @brief 二元推导检测实现
 * 国密规范 GM/T 0005-2021: k=3, k=7
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tests.h"
#include "utils.h"

static const char *name_for_k(int k)
{
    static char buf_k3[64];
    static char buf_k7[64];
    static char buf_other[64];
    if (k == 3) {
        snprintf(buf_k3, sizeof(buf_k3), "二元推导检测 (k=%d)", k);
        return buf_k3;
    }
    if (k == 7) {
        snprintf(buf_k7, sizeof(buf_k7), "二元推导检测 (k=%d)", k);
        return buf_k7;
    }
    snprintf(buf_other, sizeof(buf_other), "二元推导检测 (k=%d)", k);
    return buf_other;
}

/**
 * @brief 二元推导检测
 * 国密规范 GM/T 0005-2021: k=3, k=7
 */
TestStatus test_binary_derivative(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config)
{
    int k, deriv_len;
    double S_n, V_n, p_value, q_value;
    uint8_t *deriv_data;
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    /* 获取推导阶数k，默认为3 */
    k = config->derivate_order > 0 ? config->derivate_order : 3;
    
    /* 检查最小长度要求 */
    deriv_len = n - k;
    if (deriv_len <= 0) {
        if (k == 3) {
            result->id = "binary_derivative-k=3";
        } else {
            result->id = "binary_derivative-k=7";
        }
        result->name = name_for_k(k);
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足";
        return TEST_ERROR;
    }
    
    /* 分配推导序列缓冲区 */
    deriv_data = (uint8_t *)malloc(deriv_len);
    if (deriv_data == NULL) {
        if (k == 3) {
            result->id = "binary_derivative-k=3";
        } else {
            result->id = "binary_derivative-k=7";
        }
        result->name = name_for_k(k);
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "内存分配失败";
        return TEST_ERROR;
    }
    
    /* 执行k级推导：y_k(i) = x(i) XOR x(i+1) XOR ... XOR x(i+k) */
    for (int i = 0; i < deriv_len; i++) {
        int bit = GET_BIT(data, i);
        for (int j = 1; j <= k; j++) {
            bit ^= GET_BIT(data, i + j);
        }
        deriv_data[i] = bit;
    }
    
    /* 对推导序列执行单比特频数检测 */
    S_n = 0.0;
    for (int i = 0; i < deriv_len; i++) {
        S_n += (deriv_data[i] == 1) ? 1.0 : -1.0;
    }
    
    V_n = S_n / sqrt((double)deriv_len);
    
    /* 计算p值和q值 */
    p_value = math_erfc(fabs(V_n) / sqrt(2.0));
    q_value = math_erfc(V_n / sqrt(2.0)) / 2.0;
    
    /* 设置结果 */
    if (k == 3) {
        result->id = "binary_derivative-k=3";
    } else if (k == 7) {
        result->id = "binary_derivative-k=7";
    } else {
        result->id = "binary_derivative-k=3";
    }
    result->name = name_for_k(k);
    result->p_value = p_value;
    result->q_value = q_value;
    result->stat_value = V_n;
    
    /* 释放推导序列内存 */
    free(deriv_data);
    
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
 * @brief 二元推导检测 (k=3)
 */
TestStatus test_binary_derivative_k3(const BitData *data, int n, 
                                    TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.derivate_order = 3;
    return test_binary_derivative(data, n, result, &cfg);
}

/**
 * @brief 二元推导检测 (k=7)
 */
TestStatus test_binary_derivative_k7(const BitData *data, int n, 
                                    TestResult *result, const TestConfig *config)
{
    TestConfig cfg = *config;
    cfg.derivate_order = 7;
    return test_binary_derivative(data, n, result, &cfg);
}