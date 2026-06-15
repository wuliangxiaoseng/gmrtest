/**
 * @file tests_stub.c
 * @brief 桩实现：保留 NIST 规范但国密规范不要求的检测项
 * 国密 GM/T 0005-2021 不要求以下项，留待后续低优先级实现：
 *   - test_non_overlapping_template
 *   - test_overlapping_template
 *   - test_random_excursions
 *   - test_random_excursions_variant
 *   - test_binary_derivate (旧名，新接口为 binary_derivative-k=*)
 * @author gmrtest
 * @date 2024
 */

#include "tests.h"

TestStatus test_non_overlapping_template(const BitData *data, int n, 
                                         TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_NON_OVERLAPPING_TMPL_STR;
    result->name = "非重叠模板匹配";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_overlapping_template(const BitData *data, int n, 
                                     TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_OVERLAPPING_TMPL_STR;
    result->name = "重叠模板匹配";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_random_excursions(const BitData *data, int n, 
                                  TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_RANDOM_EXCURSIONS_STR;
    result->name = "随机游程检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_random_excursions_variant(const BitData *data, int n, 
                                          TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_RANDOM_EXCURSIONS_VAR_STR;
    result->name = "随机游程变体检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_binary_derivate(const BitData *data, int n, 
                                TestResult *result, const TestConfig *config)
{
    /* 旧接口保留兼容；国密项请使用 test_binary_derivative_k3/k7 */
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_BINARY_DERIVATE_STR;
    result->name = "二元推导检测(旧)";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "兼容旧接口";
    return TEST_SUCCESS;
}
