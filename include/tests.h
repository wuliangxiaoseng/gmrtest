/**
 * @file tests.h
 * @brief 测试函数声明
 * @author gmrtest
 * @date 2024
 */

#ifndef GMRTEST_TESTS_H
#define GMRTEST_TESTS_H

#include "types.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 核心统计检测函数
 * ============================================================================ */

/**
 * @brief 单比特频数检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_frequency(const BitData *data, int n, 
                          TestResult *result, const TestConfig *config);

/**
 * @brief 块内频数检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 block_length)
 * @return 测试状态
 */
TestStatus test_block_frequency(const BitData *data, int n, 
                                TestResult *result, const TestConfig *config);

/**
 * @brief 累加和检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出 (包含前向和后向两个p值)
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_cumulative_sums(const BitData *data, int n, 
                                TestResult *result, const TestConfig *config);

/**
 * @brief 游程总数检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_runs(const BitData *data, int n, 
                     TestResult *result, const TestConfig *config);

/**
 * @brief 块内最长1游程检测
 * 国密规范 GM/T 0005-2021: m=10000
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 block_length)
 * @return 测试状态
 */
TestStatus test_longest_run_ones(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config);

/**
 * @brief 块内最长0游程检测
 * 国密规范 GM/T 0005-2021: m=10000（2021版新增）
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 block_length)
 * @return 测试状态
 */
TestStatus test_longest_run_zeros(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config);

/**
 * @brief 矩阵秩检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_rank(const BitData *data, int n, 
                     TestResult *result, const TestConfig *config);

/**
 * @brief 离散傅立叶变换检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_dft(const BitData *data, int n, 
                    TestResult *result, const TestConfig *config);

/**
 * @brief 非重叠模板匹配检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 template_length)
 * @return 测试状态
 */
TestStatus test_non_overlapping_template(const BitData *data, int n, 
                                         TestResult *result, const TestConfig *config);

/**
 * @brief 重叠模板匹配检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 template_length)
 * @return 测试状态
 */
TestStatus test_overlapping_template(const BitData *data, int n, 
                                     TestResult *result, const TestConfig *config);

/**
 * @brief 通用统计检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_universal(const BitData *data, int n, 
                          TestResult *result, const TestConfig *config);

/**
 * @brief 近似熵检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 block_length)
 * @return 测试状态
 */
TestStatus test_approximate_entropy(const BitData *data, int n, 
                                    TestResult *result, const TestConfig *config);

/**
 * @brief 随机游程检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_random_excursions(const BitData *data, int n, 
                                  TestResult *result, const TestConfig *config);

/**
 * @brief 随机游程变体检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_random_excursions_variant(const BitData *data, int n, 
                                          TestResult *result, const TestConfig *config);

/**
 * @brief 线性复杂度检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 block_length)
 * @return 测试状态
 */
TestStatus test_linear_complexity(const BitData *data, int n, 
                                  TestResult *result, const TestConfig *config);

/**
 * @brief 二元推导检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 derivate_order)
 * @return 测试状态
 */
TestStatus test_binary_derivate(const BitData *data, int n, 
                                TestResult *result, const TestConfig *config);

/**
 * @brief 自相关检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置 (使用 correlation_delay)
 * @return 测试状态
 */
TestStatus test_self_correlation(const BitData *data, int n, 
                                 TestResult *result, const TestConfig *config);

/**
 * @brief 扑克检测
 * 国密规范 GM/T 0005-2021 要求: m=4, m=8（两个独立检测项）
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param results 测试结果输出数组（需至少2个元素）
 * @param max_results 最大结果数量
 * @param config 测试配置
 * @return 实际生成的结果数量，失败返回-1
 */
int test_poker(const BitData *data, int n, TestResult *results, int max_results,
               const TestConfig *config);

/**
 * @brief 重叠子序列检测
 * 国密规范 GM/T 0005-2021 要求: m=3, m=5（各返回2个p值，共4项）
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param results 测试结果输出数组（需至少4个元素）
 * @param max_results 最大结果数量
 * @param config 测试配置
 * @return 实际生成的结果数量，失败返回-1
 */
int test_serial(const BitData *data, int n, TestResult *results, int max_results,
               const TestConfig *config);

/**
 * @brief 游程分布检测
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置
 * @return 测试状态
 */
TestStatus test_runs_distribution(const BitData *data, int n, 
                                  TestResult *result, const TestConfig *config);

/* ============================================================================
 * 测试管理函数
 * ============================================================================ */

/**
 * @brief 获取所有可用测试项
 * @return 测试项数组
 */
const TestItem* get_all_tests(void);

/**
 * @brief 获取测试项数量
 * @return 测试数量
 */
int get_test_count(void);

/**
 * @brief 根据ID获取测试项
 * @param id 测试ID
 * @return 测试项指针，未找到返回NULL
 */
const TestItem* get_test_by_id(const char *id);

#ifdef __cplusplus
}
#endif

#endif /* GMRTEST_TESTS_H */
