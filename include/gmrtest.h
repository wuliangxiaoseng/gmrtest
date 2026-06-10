/**
 * @file gmrtest.h
 * @brief gmrtest 主API头文件
 * @author gmrtest
 * @date 2024
 * 
 * 国密随机数检测工具 (GuoMi Random TEST)
 * 实现国密规范要求的随机性统计检测算法
 */

#ifndef GMRTEST_H
#define GMRTEST_H

#include "types.h"
#include "config.h"
#include "tests.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 版本信息
 * ============================================================================ */

/**
 * @brief 获取版本字符串
 * @return 版本字符串
 */
const char* gmrtest_version(void);

/**
 * @brief 获取版本号
 * @param major 主版本号输出
 * @param minor 次版本号输出
 * @param patch 补丁版本号输出
 */
void gmrtest_version_numbers(int *major, int *minor, int *patch);

/* ============================================================================
 * 主API函数
 * ============================================================================ */

/**
 * @brief 运行单个测试
 * @param test_id 测试ID
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param result 测试结果输出
 * @param config 测试配置（NULL使用默认配置）
 * @return 测试状态
 */
TestStatus gmrtest_run_single(const char *test_id, const BitData *data, int n,
                              TestResult *result, const TestConfig *config);

/**
 * @brief 运行多个测试
 * @param test_ids 测试ID数组
 * @param test_count 测试数量
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param summary 测试结果汇总输出
 * @param config 测试配置（NULL使用默认配置）
 * @return 成功返回0，失败返回-1
 */
int gmrtest_run_multiple(const char **test_ids, int test_count,
                         const BitData *data, int n,
                         TestSummary *summary, const TestConfig *config);

/**
 * @brief 运行所有测试
 * @param data 比特数据
 * @param n 序列长度（比特数）
 * @param summary 测试结果汇总输出
 * @param config 测试配置（NULL使用默认配置）
 * @return 成功返回0，失败返回-1
 */
int gmrtest_run_all(const BitData *data, int n,
                    TestSummary *summary, const TestConfig *config);

/**
 * @brief 从文件运行测试
 * @param filename 输入文件名
 * @param test_ids 测试ID数组（NULL表示运行所有测试）
 * @param test_count 测试数量（0表示运行所有测试）
 * @param summary 测试结果汇总输出
 * @param config 测试配置（NULL使用默认配置）
 * @return 成功返回读取的比特数，失败返回-1
 */
int gmrtest_run_file(const char *filename,
                     const char **test_ids, int test_count,
                     TestSummary *summary, const TestConfig *config);

/* ============================================================================
 * 配置函数
 * ============================================================================ */

/**
 * @brief 获取默认配置
 * @param config 配置结构输出
 */
void gmrtest_default_config(TestConfig *config);

/**
 * @brief 验证配置有效性
 * @param config 配置结构
 * @return 有效返回1，无效返回0
 */
int gmrtest_validate_config(const TestConfig *config);

/* ============================================================================
 * 信息函数
 * ============================================================================ */

/**
 * @brief 打印所有可用测试
 * @param fp 输出文件指针
 */
void gmrtest_list_tests(FILE *fp);

/**
 * @brief 打印帮助信息
 * @param fp 输出文件指针
 */
void gmrtest_print_help(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* GMRTEST_H */
