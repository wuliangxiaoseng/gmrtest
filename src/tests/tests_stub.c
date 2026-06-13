/**
 * @file tests_stub.c
 * @brief 测试函数桩实现（待完善）
 * @author gmrtest
 * @date 2024
 */

#include "tests.h"

TestStatus test_cumulative_sums(const BitData *data, int n, 
                                TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_CUMULATIVE_SUMS_STR;
    result->name = "累加和检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_longest_run_ones(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_LONGEST_RUN_ONES_STR;
    result->name = "最长1游程检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_rank(const BitData *data, int n, 
                     TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_RANK_STR;
    result->name = "矩阵秩检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_dft(const BitData *data, int n, 
                    TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_DFT_STR;
    result->name = "离散傅立叶检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

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

TestStatus test_universal(const BitData *data, int n, 
                          TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_UNIVERSAL_STR;
    result->name = "通用统计检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_approximate_entropy(const BitData *data, int n, 
                                    TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_APPROXIMATE_ENTROPY_STR;
    result->name = "近似熵检测";
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

TestStatus test_linear_complexity(const BitData *data, int n, 
                                  TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_LINEAR_COMPLEXITY_STR;
    result->name = "线性复杂度检测";
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
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_BINARY_DERIVATE_STR;
    result->name = "二元推导检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}

TestStatus test_self_correlation(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config)
{
    (void)data;
    (void)config;
    if (result == NULL || n <= 0) return TEST_ERROR;
    result->id = TEST_ID_SELF_CORRELATION_STR;
    result->name = "自相关检测";
    result->p_value = 0.5;
    result->q_value = 0.5;
    result->stat_value = 0.0;
    result->status = TEST_SUCCESS;
    result->message = "待实现";
    return TEST_SUCCESS;
}
