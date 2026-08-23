/**
 * @file types.h
 * @brief 基础类型定义
 * @author gmrtest
 * @date 2024
 */

#ifndef GMRTEST_TYPES_H
#define GMRTEST_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * 比特存储模式配置
 * 
 * BIT_PACKING=0 (默认) - 字节/比特模式
 * BIT_PACKING=1 - 位压缩模式
 * ============================================================================ */

/**
 * @brief 比特数据类型
 * 根据 BIT_PACKING 模式定义不同的类型：
 * - BIT_PACKING=0: BitData = uint8_t，每个比特占用1字节
 * - BIT_PACKING=1: BitData = uint8_t，8个比特压缩到1字节
 */
typedef uint8_t BitData;

/**
 * @brief 获取比特值的统一宏
 * 根据 BIT_PACKING 模式选择不同的实现：
 * - BIT_PACKING=0: 直接返回字节值（假设0或1）
 * - BIT_PACKING=1: 进行位运算提取比特
 */
#if BIT_PACKING
#define GET_BIT(data, index)             (((data)[(index) >> 3] >> ((index) & 7)) & 1)
#else
#define GET_BIT(data, index)             ((data)[index])
#endif

/**
 * @brief 设置比特值的统一宏
 */
#if BIT_PACKING
#define SET_BIT(data, index, value) do { \
    int _byte = (index) >> 3; \
    int _bit = (index) & 7; \
    if (value) { \
        (data)[_byte] |= (1 << _bit); \
    } else { \
        (data)[_byte] &= ~(1 << _bit); \
    } \
} while (0)
#else
#define SET_BIT(data, index, value)      ((data)[index] = (value))
#endif

/* ============================================================================
 * 比特序列描述符
 * ============================================================================ */

/**
 * @brief 比特序列描述符
 * 用于描述一段比特数据
 */
typedef struct {
    BitData *data;       /**< 比特数据 */
    int length;         /**< 比特序列长度（比特数） */
    int allocated;      /**< 分配的字节数 */
    bool owns_data;     /**< 是否拥有数据所有权 */
} BitSequence;

/* ============================================================================
 * 位操作函数声明
 * ============================================================================ */

/**
 * @brief 获取指定位置的比特值
 * @param data 比特数据
 * @param index 比特索引（从0开始）
 * @return 比特值 (0 或 1)
 */
static inline int bit_get(const BitData *data, int index) {
    return GET_BIT(data, index);
}

/**
 * @brief 设置指定位置的比特值
 * @param data 比特数据
 * @param index 比特索引（从0开始）
 * @param value 比特值 (0 或 1)
 */
static inline void bit_set(BitData *data, int index, int value) {
    SET_BIT(data, index, value);
}

/**
 * @brief 复制比特序列
 * @param dest 目标序列
 * @param src 源序列
 * @param n 要复制的比特数
 */
void bit_copy(BitData *dest, const BitData *src, int n);

/**
 * @brief 释放比特序列
 * @param seq 比特序列指针
 */
void bit_sequence_free(BitSequence *seq);

/* ============================================================================
 * 测试结果状态
 * ============================================================================ */

/**
 * @brief 测试结果状态
 */
typedef enum {
    TEST_SUCCESS = 0,   /**< 测试通过 */
    TEST_FAILURE = 1,   /**< 测试失败 */
    TEST_ERROR   = 2    /**< 测试错误（参数无效等） */
} TestStatus;

/**
 * @brief 单个测试结果结构
 */
typedef struct {
    const char *name;       /**< 测试名称 */
    const char *id;         /**< 测试ID标识 */
    double p_value;         /**< P值（一级检测：通过率判定） */
    double q_value;         /**< Q值（二级检测：均匀性判定，国密规范新增） */
    double stat_value;      /**< 统计量值 */
    TestStatus status;      /**< 测试状态 */
    const char *message;    /**< 附加消息（可选） */
} TestResult;

/**
 * @brief 测试配置参数
 */
typedef struct {
    int n;                  /**< 序列长度 */
    int block_length;       /**< 块长度（通用） */
    int template_length;    /**< 模板长度 */
    int derivate_order;     /**< 推导阶数 */
    int correlation_delay;  /**< 自相关延迟 */
    int cusum_mode;         /**< 累加和方向：0=forward, 1=backward */
    int num_threads;        /**< 并行线程数：1=单线程, >1=并行, <=0=自动(取环境变量,默认1) */
    double alpha;           /**< 显著性水平 */
} TestConfig;

/**
 * @brief 所有测试结果汇总
 */
typedef struct {
    int total_tests;        /**< 总测试数 */
    int passed_tests;       /**< 通过测试数 */
    int failed_tests;       /**< 失败测试数 */
    int error_tests;        /**< 错误测试数 */
    TestResult *results;    /**< 结果数组 */
    int results_count;      /**< 结果数量 */
} TestSummary;

/**
 * @brief 测试函数类型（返回单个结果）
 */
typedef TestStatus (*TestFunc)(const BitData *data, int n, 
                               TestResult *result, const TestConfig *config);

/**
 * @brief 测试函数类型（返回多个结果）
 */
typedef int (*TestMultiFunc)(const BitData *data, int n, 
                             TestResult *results, int max_results,
                             const TestConfig *config);

/**
 * @brief 测试项定义
 */
typedef struct {
    const char *id;           /**< 测试唯一标识 */
    const char *name;         /**< 测试名称（中文） */
    const char *description;  /**< 测试描述 */
    TestFunc func;            /**< 测试函数（单结果） */
    TestMultiFunc multi_func; /**< 测试函数（多结果，NULL表示单结果） */
    int min_length;           /**< 最小序列长度要求 */
} TestItem;

/* ============================================================================
 * 常量定义
 * ============================================================================ */

#define MAX_RESULTS                     64  /**< 最大测试结果数量 */

#define GMRTEST_VERSION_MAJOR 1
#define GMRTEST_VERSION_MINOR 0
#define GMRTEST_VERSION_PATCH 0
#define GMRTEST_VERSION "1.0.0"

/* 数值比较宏 */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* 数值判断宏 */
#define IS_NEGATIVE(x)     ((x) < 0.0)
#define IS_POSITIVE(x)     ((x) > 0.0)
#define IS_ZERO(x)         ((x) == 0.0)
#define IS_GREATER_THAN_ONE(x) ((x) > 1.0)

#endif /* GMRTEST_TYPES_H */
