/**
 * @file utils.h
 * @brief 工具函数声明
 * @author gmrtest
 * @date 2024
 */

#ifndef GMRTEST_UTILS_H
#define GMRTEST_UTILS_H

#include "types.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 数学函数
 * ============================================================================ */

/**
 * @brief 互补误差函数 erfc
 * @param x 输入值
 * @return erfc(x)
 */
double math_erfc(double x);

/**
 * @brief 不完全伽马函数的补函数 igamc
 * @param a 参数a
 * @param x 参数x
 * @return igamc(a, x) = 1 - igam(a, x)
 */
double math_igamc(double a, double x);

/**
 * @brief 对数伽马函数 lgam
 * @param x 输入值
 * @return ln(Gamma(x))
 */
double math_lgam(double x);

/**
 * @brief 标准正态分布累积函数
 * @param x 输入值
 * @return Phi(x) = integral from -inf to x of (1/sqrt(2*pi)) * exp(-t^2/2) dt
 */
double math_normal(double x);

/**
 * @brief 伽马函数
 * @param x 输入值
 * @return Gamma(x)
 */
double math_gamma(double x);

/* ============================================================================
 * 比特序列处理
 * ============================================================================ */

/**
 * @brief 将字节数组转换为比特序列
 * @param bytes 输入字节数组
 * @param byte_len 字节数组长度
 * @param bits 输出比特序列（需预先分配）
 * @return 成功返回转换后的比特数，失败返回-1
 */
int bytes_to_bits(const uint8_t *bytes, int byte_len, BitData *bits);

/**
 * @brief 从文件读取比特序列
 * @param filename 文件名
 * @param bits 输出比特序列缓冲区
 * @param max_bits 最大比特数
 * @return 成功返回读取的比特数，失败返回-1
 */
int read_bits_from_file(const char *filename, BitData *bits, int max_bits);

/**
 * @brief 从文件读取比特序列（自动分配内存）
 * @param filename 文件名
 * @param bits 输出比特序列指针（函数内分配）
 * @return 成功返回读取的比特数，失败返回-1
 */
int read_bits_from_file_alloc(const char *filename, BitData **bits);

/**
 * @brief 打印比特序列（调试用）
 * @param bits 比特数据
 * @param n 序列长度
 * @param max_print 最大打印数量
 */
void print_bits(const BitData *bits, int n, int max_print);

/* ============================================================================
 * 输出格式化
 * ============================================================================ */

/**
 * @brief 将单个文件的测试结果输出为单个JSON对象（结尾不含换行，供批量数组拼接）
 * @param fp 输出文件指针
 * @param summary 测试结果汇总
 * @param filename 输入文件名
 * @return 成功返回0，失败返回-1
 */
int output_json_object(FILE *fp, const TestSummary *summary, const char *filename);

/**
 * @brief 将测试结果输出为JSON格式（单文件模式，结尾补换行）
 * @param fp 输出文件指针
 * @param summary 测试结果汇总
 * @param filename 输入文件名
 * @return 成功返回0，失败返回-1
 */
int output_json(FILE *fp, const TestSummary *summary, const char *filename);

/**
 * @brief 将测试结果输出为文本格式
 * @param fp 输出文件指针
 * @param summary 测试结果汇总
 * @param filename 输入文件名（非NULL时打印文件头）
 * @return 成功返回0，失败返回-1
 */
int output_text(FILE *fp, const TestSummary *summary, const char *filename);

/**
 * @brief 输出CSV表头（批量模式一次性打印表头用）
 * @param fp 输出文件指针
 * @param has_filename 是否包含 filename 列
 */
void output_csv_header(FILE *fp, int has_filename);

/**
 * @brief 输出CSV数据行（不含表头）
 * @param fp 输出文件指针
 * @param summary 测试结果汇总
 * @param filename 输入文件名（非 NULL 时作为首列）
 */
void output_csv_rows(FILE *fp, const TestSummary *summary, const char *filename);

/**
 * @brief 将测试结果输出为CSV格式（单文件模式：表头+数据）
 * @param fp 输出文件指针
 * @param summary 测试结果汇总
 * @param filename 输入文件名（作为首列）
 * @return 成功返回0，失败返回-1
 */
int output_csv(FILE *fp, const TestSummary *summary, const char *filename);

/* ============================================================================
 * 字符串处理
 * ============================================================================ */

/**
 * @brief 解析测试列表字符串
 * @param str 输入字符串（逗号分隔）
 * @param test_ids 输出测试ID数组
 * @param max_ids 最大ID数量
 * @return 成功返回解析的ID数量，失败返回-1
 */
int parse_test_list(const char *str, char **test_ids, int max_ids);

/**
 * @brief 去除字符串首尾空白
 * @param str 输入字符串
 * @return 处理后的字符串
 */
char* str_trim(char *str);

/* ============================================================================
 * 内存管理
 * ============================================================================ */

/**
 * @brief 初始化测试结果数组
 * @param count 结果数量
 * @return 分配的结果数组指针
 */
TestResult* results_alloc(int count);

/**
 * @brief 释放测试结果数组
 * @param results 结果数组
 */
void results_free(TestResult *results);

/**
 * @brief 初始化测试汇总结构
 * @param summary 汇总结构指针
 * @param max_results 最大结果数
 * @return 成功返回0，失败返回-1
 */
int summary_init(TestSummary *summary, int max_results);

/**
 * @brief 释放测试汇总结构
 * @param summary 汇总结构指针
 */
void summary_free(TestSummary *summary);

/* ============================================================================
 * 错误处理
 * ============================================================================ */

/**
 * @brief 获取错误信息
 * @param status 测试状态
 * @return 错误信息字符串
 */
const char* get_error_message(TestStatus status);

/* ============================================================================
 * 时间测量
 * ============================================================================ */

/**
 * @brief 获取当前时间戳（毫秒）
 * @return 时间戳
 */
long long get_timestamp_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* GMRTEST_UTILS_H */
