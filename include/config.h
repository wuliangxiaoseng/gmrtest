/**
 * @file config.h
 * @brief 配置常量定义
 * @author gmrtest
 * @date 2024
 */

#ifndef GMRTEST_CONFIG_H
#define GMRTEST_CONFIG_H

#include "types.h"

/* ============================================================================
 * 显著性水平
 * ============================================================================ */

/** 默认显著性水平 (国密规范推荐值) */
#define DEFAULT_ALPHA           0.01

/** 显著性水平范围检查 */
#define ALPHA_MIN               0.0
#define ALPHA_MAX               1.0

/* ============================================================================
 * 测试ID定义 (与国密规范对应)
 * 整数ID用于逻辑判断，字符串ID用于输出标识
 * ============================================================================ */

/* 整数ID */
#define TEST_ID_FREQUENCY               1
#define TEST_ID_BLOCK_FREQUENCY         2
#define TEST_ID_CUMULATIVE_SUMS         3
#define TEST_ID_RUNS                    4
#define TEST_ID_LONGEST_RUN_ONES        5
#define TEST_ID_RANK                    6
#define TEST_ID_DFT                     7
#define TEST_ID_NON_OVERLAPPING_TMPL    8
#define TEST_ID_OVERLAPPING_TMPL        9
#define TEST_ID_UNIVERSAL               10
#define TEST_ID_APPROXIMATE_ENTROPY     11
#define TEST_ID_RANDOM_EXCURSIONS       12
#define TEST_ID_RANDOM_EXCURSIONS_VAR   13
#define TEST_ID_SERIAL                  14
#define TEST_ID_LINEAR_COMPLEXITY       15
#define TEST_ID_BINARY_DERIVATE         16
#define TEST_ID_SELF_CORRELATION        17
#define TEST_ID_POKER                   18
#define TEST_ID_RUNS_DISTRIBUTION       19

/* 字符串ID */
#define TEST_ID_FREQUENCY_STR               "frequency"
#define TEST_ID_BLOCK_FREQUENCY_STR         "block_frequency"
#define TEST_ID_CUMULATIVE_SUMS_STR         "cumulative_sums"
#define TEST_ID_RUNS_STR                    "runs"
#define TEST_ID_LONGEST_RUN_ONES_STR        "longest_run_ones"
#define TEST_ID_RANK_STR                    "rank"
#define TEST_ID_DFT_STR                     "dft"
#define TEST_ID_NON_OVERLAPPING_TMPL_STR    "non_overlapping_template"
#define TEST_ID_OVERLAPPING_TMPL_STR        "overlapping_template"
#define TEST_ID_UNIVERSAL_STR               "universal"
#define TEST_ID_APPROXIMATE_ENTROPY_STR     "approximate_entropy"
#define TEST_ID_RANDOM_EXCURSIONS_STR       "random_excursions"
#define TEST_ID_RANDOM_EXCURSIONS_VAR_STR   "random_excursions_variant"
#define TEST_ID_SERIAL_STR                  "serial"
#define TEST_ID_LINEAR_COMPLEXITY_STR       "linear_complexity"
#define TEST_ID_BINARY_DERIVATE_STR         "binary_derivate"
#define TEST_ID_SELF_CORRELATION_STR        "self_correlation"
#define TEST_ID_POKER_STR                   "poker"
#define TEST_ID_RUNS_DISTRIBUTION_STR       "runs_distribution"

/** 测试总数 */
#define TEST_COUNT                      19

/* ============================================================================
 * 默认参数配置
 * ============================================================================ */

/* 块内频数检测 - 国密规范 GM/T 0005-2021 要求 m=10000 */
#define DEFAULT_BLOCK_FREQUENCY_M       10000

/* 非重叠模板匹配 */
#define DEFAULT_NON_OVERLAPPING_M       9
#define MAX_TEMPLATE_LENGTH             21

/* 重叠模板匹配 */
#define DEFAULT_OVERLAPPING_M           9

/* 重叠子序列检测 */
#define DEFAULT_SERIAL_M                16

/* 线性复杂度检测 */
#define DEFAULT_LINEAR_COMPLEXITY_M     500

/* 近似熵检测 */
#define DEFAULT_APPROXIMATE_ENTROPY_M   10

/* 二元推导检测 */
#define DEFAULT_DERIVATE_ORDER          3

/* 自相关检测 */
#define DEFAULT_CORRELATION_DELAY       1

/* 扑克检测 */
#define DEFAULT_POKER_M                 8
#define MAX_POKER_M                     8

/* ============================================================================
 * 最小序列长度要求
 * ============================================================================ */

#define MIN_LENGTH_FREQUENCY            100
#define MIN_LENGTH_BLOCK_FREQUENCY      100
#define MIN_LENGTH_CUMULATIVE_SUMS      100
#define MIN_LENGTH_RUNS                 100
#define MIN_LENGTH_LONGEST_RUN_ONES     128
#define MIN_LENGTH_RANK                 1024
#define MIN_LENGTH_DFT                  1000
#define MIN_LENGTH_NON_OVERLAPPING      100000
#define MIN_LENGTH_OVERLAPPING          8256
#define MIN_LENGTH_UNIVERSAL            387840
#define MIN_LENGTH_APPROXIMATE_ENTROPY  100
#define MIN_LENGTH_SERIAL               100
#define MIN_LENGTH_LINEAR_COMPLEXITY    500
#define MIN_LENGTH_RANDOM_EXCURSIONS    10000
#define MIN_LENGTH_RANDOM_EXCURSIONS_VAR 10000
#define MIN_LENGTH_BINARY_DERIVATE      100
#define MIN_LENGTH_SELF_CORRELATION     100
#define MIN_LENGTH_POKER                100
#define MIN_LENGTH_RUNS_DISTRIBUTION    100

/* ============================================================================
 * 内存和缓冲区配置
 * ============================================================================ */

/** 最大模板数量 */
#define MAX_NUM_OF_TEMPLATES            148

/** 最大周期数 (随机游程检测) */
#define MAX_CYCLES                      10000

/** 输出缓冲区大小 */
#define OUTPUT_BUFFER_SIZE              4096

/** 文件路径最大长度 */
#define MAX_PATH_LENGTH                 1024

/* ============================================================================
 * 比特存储模式选择
 * 通过编译宏控制，支持两种存储方案：
 * 
 * 1. BIT_PACKING=0 (默认) - 字节/比特模式
 *    - 每个比特占用1个字节
 *    - 优点：访问速度快，代码简单，兼容NIST原始实现
 *    - 缺点：内存占用大（膨胀8倍）
 * 
 * 2. BIT_PACKING=1 - 位压缩模式
 *    - 8个比特压缩到1个字节
 *    - 优点：内存效率高（节省87.5%）
 *    - 缺点：访问需要位运算，稍慢
 * 
 * 编译时通过 -DBIT_PACKING=1 启用位压缩模式
 * ============================================================================ */

#ifndef BIT_PACKING
#define BIT_PACKING                     0  /* 默认使用字节/比特模式 */
#endif

/** 位压缩模式下的字节计算宏 */
#if BIT_PACKING
#define BITS_TO_BYTES(n)                (((n) + 7) >> 3)
#else
#define BITS_TO_BYTES(n)                (n)
#endif

/* ============================================================================
 * 调试选项
 * ============================================================================ */

#ifdef DEBUG
#define GMRTEST_DEBUG 1
#else
#define GMRTEST_DEBUG 0
#endif

/** 调试打印宏 */
#if GMRTEST_DEBUG
#include <stdio.h>
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) ((void)0)
#endif

#endif /* GMRTEST_CONFIG_H */
