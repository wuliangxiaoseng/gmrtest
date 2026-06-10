/**
 * @file frequency.c
 * @brief 单比特频数检测实现
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include "tests.h"
#include "utils.h"

TestStatus test_frequency(const BitData *data, int n, 
                          TestResult *result, const TestConfig *config)
{
    int i;
    double sum, v_obs, s_obs, p_value, q_value;
    double sqrt2 = 1.41421356237309504880;
    
    if (data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    sum = 0.0;
    for (i = 0; i < n; i++) {
        sum += 2.0 * (double)GET_BIT(data, i) - 1.0;
    }
    
    /* 国密规范 GM/T 0005-2021:
     * P_value = erfc(|V| / √2)  用于一级检测（通过率）
     * Q_value = erfc(V / √2)   用于二级检测（均匀性），不取绝对值
     */
    v_obs = sum / sqrt((double)n);      /* 原始值，不取绝对值 */
    s_obs = fabs(v_obs);                 /* 取绝对值 */
    
    p_value = math_erfc(s_obs / sqrt2);  /* 一级检测 P值 */
    q_value = math_erfc(v_obs / sqrt2);  /* 二级检测 Q值 */
    
    result->id = TEST_ID_FREQUENCY_STR;
    result->name = "单比特频数检测";
    result->p_value = p_value;
    result->q_value = q_value;
    result->stat_value = s_obs;
    
    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}
