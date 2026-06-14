/**
 * @file longest_run.c
 * @brief 块内最大1游程检测和块内最大0游程检测实现
 * 国密规范 GM/T 0005-2021: m=10000
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include "tests.h"
#include "utils.h"

/**
 * @brief 块内最长1游程检测（基于NIST SP 800-22和GM/T 0005-2021）
 */
TestStatus test_longest_run_ones(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config)
{
    double pval, chi2;
    int run, v_n_obs, N, i, j, K, M;
    unsigned int nu[7] = { 0, 0, 0, 0, 0, 0, 0 };
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    /* 根据数据长度选择参数 */
    if (n < 128) {
        result->id = TEST_ID_LONGEST_RUN_ONES_STR;
        result->name = "块内最大1游程检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足";
        return TEST_ERROR;
    }
    
    /* 使用国密规范要求的 m=10000（适用于 n >= 750000） */
    K = 6;
    M = config->block_length > 0 ? config->block_length : 10000;
    
    double pi[7] = {
        0.0882,  /* <=10 */
        0.2092,  /* 11 */
        0.2483,  /* 12 */
        0.1933,  /* 13 */
        0.1208,  /* 14 */
        0.0675,  /* 15 */
        0.0727   /* >=16 */
    };
    
    int V[7] = {10, 11, 12, 13, 14, 15, 16};
    
    N = n / M;
    if (N < 1) {
        result->id = TEST_ID_LONGEST_RUN_ONES_STR;
        result->name = "块内最大1游程检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足";
        return TEST_ERROR;
    }
    
    /* 遍历每个块，找到最长1游程 */
    for (i = 0; i < N; i++) {
        v_n_obs = 0;
        run = 0;
        for (j = 0; j < M; j++) {
            if (GET_BIT(data, i * M + j) == 1) {
                run++;
                if (run > v_n_obs) {
                    v_n_obs = run;
                }
            } else {
                run = 0;
            }
        }
        
        /* 统计频率 */
        if (v_n_obs < V[0]) {
            nu[0]++;
        }
        for (j = 0; j <= K; j++) {
            if (v_n_obs == V[j]) {
                nu[j]++;
            }
        }
        if (v_n_obs > V[K]) {
            nu[K]++;
        }
    }
    
    /* 计算卡方统计量 */
    chi2 = 0.0;
    for (i = 0; i <= K; i++) {
        double expected = N * pi[i];
        chi2 += pow((double)nu[i] - expected, 2.0) / expected;
    }
    
    /* 计算 p_value = igamc(K/2, chi2/2) */
    pval = math_igamc((double)K / 2.0, chi2 / 2.0);
    
    /* 设置结果 */
    result->id = TEST_ID_LONGEST_RUN_ONES_STR;
    result->name = "块内最大1游程检测";
    result->p_value = pval;
    result->q_value = pval;
    result->stat_value = chi2;
    
    if (pval < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}

/**
 * @brief 块内最长0游程检测（国密规范GM/T 0005-2021新增）
 */
TestStatus test_longest_run_zeros(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config)
{
    double pval, chi2;
    int run, v_n_obs, N, i, j, K, M;
    unsigned int nu[7] = { 0, 0, 0, 0, 0, 0, 0 };
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    /* 根据数据长度选择参数 */
    if (n < 128) {
        result->id = "longest_run_zeros";
        result->name = "块内最大0游程检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足";
        return TEST_ERROR;
    }
    
    /* 使用国密规范要求的 m=10000（适用于 n >= 750000） */
    K = 6;
    M = config->block_length > 0 ? config->block_length : 10000;
    
    double pi[7] = {
        0.0882,  /* <=10 */
        0.2092,  /* 11 */
        0.2483,  /* 12 */
        0.1933,  /* 13 */
        0.1208,  /* 14 */
        0.0675,  /* 15 */
        0.0727   /* >=16 */
    };
    
    int V[7] = {10, 11, 12, 13, 14, 15, 16};
    
    N = n / M;
    if (N < 1) {
        result->id = "longest_run_zeros";
        result->name = "块内最大0游程检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足";
        return TEST_ERROR;
    }
    
    /* 遍历每个块，找到最长0游程 */
    for (i = 0; i < N; i++) {
        v_n_obs = 0;
        run = 0;
        for (j = 0; j < M; j++) {
            if (GET_BIT(data, i * M + j) == 0) {
                run++;
                if (run > v_n_obs) {
                    v_n_obs = run;
                }
            } else {
                run = 0;
            }
        }
        
        /* 统计频率 */
        if (v_n_obs < V[0]) {
            nu[0]++;
        }
        for (j = 0; j <= K; j++) {
            if (v_n_obs == V[j]) {
                nu[j]++;
            }
        }
        if (v_n_obs > V[K]) {
            nu[K]++;
        }
    }
    
    /* 计算卡方统计量 */
    chi2 = 0.0;
    for (i = 0; i <= K; i++) {
        double expected = N * pi[i];
        chi2 += pow((double)nu[i] - expected, 2.0) / expected;
    }
    
    /* 计算 p_value = igamc(K/2, chi2/2) */
    pval = math_igamc((double)K / 2.0, chi2 / 2.0);
    
    /* 设置结果 */
    result->id = "longest_run_zeros";
    result->name = "块内最大0游程检测";
    result->p_value = pval;
    result->q_value = pval;
    result->stat_value = chi2;
    
    if (pval < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}