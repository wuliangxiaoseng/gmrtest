/**
 * @file gmrtest.c
 * @brief gmrtest API 实现
 * @author gmrtest
 * @date 2024
 */

#include <stdlib.h>
#include <string.h>
#include "gmrtest.h"
#include "tests.h"

/* ============================================================================
 * 测试项定义
 * ============================================================================ */

static TestItem all_tests[TEST_COUNT] = {
    {TEST_ID_FREQUENCY_STR,              "单比特频数检测",       "检测序列中0和1的比例",       test_frequency,              NULL,                        MIN_LENGTH_FREQUENCY},
    {TEST_ID_BLOCK_FREQUENCY_STR,        "块内频数检测",         "检测块内1的比例分布",       test_block_frequency,        NULL,                        MIN_LENGTH_BLOCK_FREQUENCY},
    {TEST_ID_CUMULATIVE_SUMS_STR,        "累加和检测",           "检测累积和的随机游走",       test_cumulative_sums,        NULL,                        MIN_LENGTH_CUMULATIVE_SUMS},
    {TEST_ID_RUNS_STR,                   "游程总数检测",         "检测游程数量分布",          test_runs,                   NULL,                        MIN_LENGTH_RUNS},
    {TEST_ID_LONGEST_RUN_ONES_STR,       "最长1游程检测",        "检测最长连续1的长度",       test_longest_run_ones,       NULL,                        MIN_LENGTH_LONGEST_RUN_ONES},
    {TEST_ID_LONGEST_RUN_ZEROS_STR,       "最长0游程检测",        "检测最长连续0的长度",       test_longest_run_zeros,      NULL,                        MIN_LENGTH_LONGEST_RUN_ONES},
    {TEST_ID_RANK_STR,                   "矩阵秩检测",           "检测矩阵秩的分布",          test_rank,                   NULL,                        MIN_LENGTH_RANK},
    {TEST_ID_DFT_STR,                    "离散傅立叶检测",       "通过FFT检测周期性",         test_dft,                    NULL,                        MIN_LENGTH_DFT},
    {TEST_ID_NON_OVERLAPPING_TMPL_STR,   "非重叠模板匹配",       "检测非重叠模板出现频率",    test_non_overlapping_template, NULL,                    MIN_LENGTH_NON_OVERLAPPING},
    {TEST_ID_OVERLAPPING_TMPL_STR,       "重叠模板匹配",         "检测重叠模板出现频率",      test_overlapping_template,   NULL,                        MIN_LENGTH_OVERLAPPING},
    {TEST_ID_UNIVERSAL_STR,              "通用统计检测",         "检测序列压缩性",            test_universal,              NULL,                        MIN_LENGTH_UNIVERSAL},
    {TEST_ID_APPROXIMATE_ENTROPY_STR,    "近似熵检测",           "检测序列复杂度",            test_approximate_entropy,    NULL,                        MIN_LENGTH_APPROXIMATE_ENTROPY},
    {TEST_ID_RANDOM_EXCURSIONS_STR,      "随机游程检测",         "检测随机游走返回次数",      test_random_excursions,      NULL,                        MIN_LENGTH_RANDOM_EXCURSIONS},
    {TEST_ID_RANDOM_EXCURSIONS_VAR_STR,  "随机游程变体检测",     "检测扩展状态访问次数",      test_random_excursions_variant, NULL,                    MIN_LENGTH_RANDOM_EXCURSIONS_VAR},
    {TEST_ID_SERIAL_STR,                 "重叠子序列检测",       "检测连续模式频率分布",      NULL,                        test_serial,                 MIN_LENGTH_SERIAL},
    {TEST_ID_LINEAR_COMPLEXITY_STR,      "线性复杂度检测",       "检测LFSR最短长度",         test_linear_complexity,      NULL,                        MIN_LENGTH_LINEAR_COMPLEXITY},
    {TEST_ID_BINARY_DERIVATE_STR,        "二元推导检测",         "检测k阶推导后随机性",       test_binary_derivate,        NULL,                        MIN_LENGTH_BINARY_DERIVATE},
    {TEST_ID_SELF_CORRELATION_STR,       "自相关检测",           "检测序列自相关性",          test_self_correlation,       NULL,                        MIN_LENGTH_SELF_CORRELATION},
    {TEST_ID_POKER_STR,                  "扑克检测",             "检测块模式分布",            NULL,                        test_poker,                  MIN_LENGTH_POKER},
    {TEST_ID_RUNS_DISTRIBUTION_STR,      "游程分布检测",         "检测游程长度分布",          test_runs_distribution,      NULL,                        MIN_LENGTH_RUNS_DISTRIBUTION}
};

/* ============================================================================
 * 版本信息
 * ============================================================================ */

const char* gmrtest_version(void)
{
    return GMRTEST_VERSION;
}

void gmrtest_version_numbers(int *major, int *minor, int *patch)
{
    if (major) *major = GMRTEST_VERSION_MAJOR;
    if (minor) *minor = GMRTEST_VERSION_MINOR;
    if (patch) *patch = GMRTEST_VERSION_PATCH;
}

/* ============================================================================
 * 配置函数
 * ============================================================================ */

void gmrtest_default_config(TestConfig *config)
{
    if (config == NULL) return;
    
    config->n = 0;
    config->block_length = DEFAULT_BLOCK_FREQUENCY_M;
    config->template_length = DEFAULT_NON_OVERLAPPING_M;
    config->derivate_order = DEFAULT_DERIVATE_ORDER;
    config->correlation_delay = DEFAULT_CORRELATION_DELAY;
    config->alpha = DEFAULT_ALPHA;
}

int gmrtest_validate_config(const TestConfig *config)
{
    if (config == NULL) return 0;
    
    if (config->alpha <= ALPHA_MIN || config->alpha >= ALPHA_MAX) return 0;
    if (config->block_length <= 0) return 0;
    if (config->template_length <= 0 || config->template_length > MAX_TEMPLATE_LENGTH) return 0;
    if (config->derivate_order <= 0) return 0;
    if (config->correlation_delay <= 0) return 0;
    
    return 1;
}

/* ============================================================================
 * 测试管理函数
 * ============================================================================ */

const TestItem* get_all_tests(void)
{
    return all_tests;
}

int get_test_count(void)
{
    return TEST_COUNT;
}

const TestItem* get_test_by_id(const char *id)
{
    if (id == NULL) return NULL;
    
    for (int i = 0; i < TEST_COUNT; i++) {
        if (strcmp(all_tests[i].id, id) == 0) {
            return &all_tests[i];
        }
    }
    return NULL;
}

/* ============================================================================
 * 主API函数
 * ============================================================================ */

TestStatus gmrtest_run_single(const char *test_id, const BitData *data, int n,
                              TestResult *result, const TestConfig *config)
{
    const TestItem *test;
    TestConfig default_config;
    
    if (test_id == NULL || data == NULL || result == NULL || n <= 0) {
        return TEST_ERROR;
    }
    
    test = get_test_by_id(test_id);
    if (test == NULL) {
        return TEST_ERROR;
    }
    
    if (n < test->min_length) {
        return TEST_ERROR;
    }
    
    if (config == NULL) {
        gmrtest_default_config(&default_config);
        config = &default_config;
    }
    
    /* 支持单结果和多结果测试 */
    if (test->multi_func != NULL) {
        /* 多结果测试，返回第一个结果 */
        int count = test->multi_func(data, n, result, 1, config);
        return (count > 0) ? result[0].status : TEST_ERROR;
    } else {
        /* 单结果测试 */
        return test->func(data, n, result, config);
    }
}

int gmrtest_run_multiple(const char **test_ids, int test_count,
                         const BitData *data, int n,
                         TestSummary *summary, const TestConfig *config)
{
    TestConfig default_config;
    int i, result_idx = 0;
    
    if (test_ids == NULL || data == NULL || summary == NULL || n <= 0) {
        return -1;
    }
    
    if (config == NULL) {
        gmrtest_default_config(&default_config);
        config = &default_config;
    }
    
    summary->total_tests = 0;
    summary->passed_tests = 0;
    summary->failed_tests = 0;
    summary->error_tests = 0;
    
    for (i = 0; i < test_count; i++) {
        const char *test_id = test_ids[i];
        const TestItem *test = NULL;
        int sub_index = -1;  /* 多结果测试的子索引 */
        
        /* 处理配置项格式的测试ID (如 "poker-m=4", "serial-m=3") */
        if (strncmp(test_id, "poker-m=", 8) == 0) {
            const char *param = test_id + 8;
            test = get_test_by_id(TEST_ID_POKER_STR);
            if (strcmp(param, "4") == 0) {
                sub_index = 0;  /* m=4 -> P1 */
            } else if (strcmp(param, "8") == 0) {
                sub_index = 1;  /* m=8 -> P2 */
            }
        } else if (strncmp(test_id, "serial-m=", 9) == 0) {
            const char *param = test_id + 9;
            test = get_test_by_id(TEST_ID_SERIAL_STR);
            if (strcmp(param, "3") == 0) {
                sub_index = 0;  /* m=3 -> P1 */
            } else if (strcmp(param, "5") == 0) {
                sub_index = 2;  /* m=5 -> P1 */
            }
        } else if (strncmp(test_id, "block_frequency-m=", 18) == 0) {
            /* 块内频数检测，参数 m=10000 */
            test = get_test_by_id(TEST_ID_BLOCK_FREQUENCY_STR);
        } else if (strncmp(test_id, "cumulative_sums-", 15) == 0) {
            /* 累加和检测，支持 forward/backward */
            test = get_test_by_id(TEST_ID_CUMULATIVE_SUMS_STR);
        } else if (strncmp(test_id, "longest_run-m=", 14) == 0) {
            /* 块内最大游程检测 */
            const char *param = test_id + 14;
            if (strcmp(param, "10000,ones") == 0) {
                test = get_test_by_id(TEST_ID_LONGEST_RUN_ONES_STR);
            } else if (strcmp(param, "10000,zeros") == 0) {
                test = get_test_by_id("longest_run_zeros");
            }
        } else if (strncmp(test_id, "approximate_entropy-m=", 21) == 0) {
            /* 近似熵检测 */
            test = get_test_by_id(TEST_ID_APPROXIMATE_ENTROPY_STR);
        } else if (strncmp(test_id, "linear_complexity-m=", 19) == 0) {
            /* 线性复杂度检测 */
            test = get_test_by_id(TEST_ID_LINEAR_COMPLEXITY_STR);
        } else if (strncmp(test_id, "binary_derivate-k=", 18) == 0) {
            /* 二元推导检测 */
            test = get_test_by_id(TEST_ID_BINARY_DERIVATE_STR);
        } else if (strncmp(test_id, "autocorrelation-d=", 18) == 0) {
            /* 自相关检测 */
            test = get_test_by_id(TEST_ID_AUTOCORRELATION_STR);
        } else {
            test = get_test_by_id(test_id);
        }
        
        if (test == NULL) {
            continue;
        }
        
        if (n < test->min_length) {
            continue;
        }
        
        if (test->multi_func != NULL) {
            TestResult temp_results[4];
            int count = test->multi_func(data, n, temp_results, 4, config);
            
            if (sub_index >= 0 && sub_index < count) {
                /* 返回指定子索引的结果 */
                summary->results[result_idx] = temp_results[sub_index];
                switch (temp_results[sub_index].status) {
                    case TEST_SUCCESS:
                        summary->passed_tests++;
                        break;
                    case TEST_FAILURE:
                        summary->failed_tests++;
                        break;
                    case TEST_ERROR:
                        summary->error_tests++;
                        break;
                }
                result_idx++;
                summary->total_tests++;
            } else if (sub_index < 0) {
                /* 返回所有结果 */
                for (int j = 0; j < count && result_idx < MAX_RESULTS; j++) {
                    summary->results[result_idx] = temp_results[j];
                    switch (temp_results[j].status) {
                        case TEST_SUCCESS:
                            summary->passed_tests++;
                            break;
                        case TEST_FAILURE:
                            summary->failed_tests++;
                            break;
                        case TEST_ERROR:
                            summary->error_tests++;
                            break;
                    }
                    result_idx++;
                }
                summary->total_tests += count;
            }
        } else {
            TestResult *result = &summary->results[result_idx];
            result->id = test->id;
            result->name = test->name;
            result->p_value = 0.0;
            result->q_value = 0.0;
            result->stat_value = 0.0;
            result->status = TEST_ERROR;
            result->message = NULL;
            
            TestStatus status = test->func(data, n, result, config);
            result->status = status;
            
            switch (status) {
                case TEST_SUCCESS:
                    summary->passed_tests++;
                    break;
                case TEST_FAILURE:
                    summary->failed_tests++;
                    break;
                case TEST_ERROR:
                    summary->error_tests++;
                    break;
            }
            result_idx++;
            summary->total_tests++;
        }
    }
    
    summary->results_count = result_idx;
    return 0;
}

int gmrtest_run_all(const BitData *data, int n,
                    TestSummary *summary, const TestConfig *config)
{
    TestConfig default_config;
    int i, result_idx;
    
    if (data == NULL || summary == NULL || n <= 0) {
        return -1;
    }
    
    if (config == NULL) {
        gmrtest_default_config(&default_config);
        config = &default_config;
    }
    
    summary->total_tests = 0;
    summary->passed_tests = 0;
    summary->failed_tests = 0;
    summary->error_tests = 0;
    result_idx = 0;
    
    for (i = 0; i < TEST_COUNT; i++) {
        const TestItem *test = &all_tests[i];
        
        /* 检查序列长度 */
        if (n < test->min_length) {
            /* 跳过长度不足的测试 */
            continue;
        }
        
        /* 支持单结果和多结果测试 */
        if (test->multi_func != NULL) {
            /* 多结果测试：扑克检测(m=4,m=8), 重叠子序列检测(m=3,m=5各2个p值) */
            TestResult temp_results[4];  /* 最多4个结果 */
            int count = test->multi_func(data, n, temp_results, 4, config);
            
            for (int j = 0; j < count && result_idx < MAX_RESULTS; j++) {
                summary->results[result_idx] = temp_results[j];
                switch (temp_results[j].status) {
                    case TEST_SUCCESS:
                        summary->passed_tests++;
                        break;
                    case TEST_FAILURE:
                        summary->failed_tests++;
                        break;
                    case TEST_ERROR:
                        summary->error_tests++;
                        break;
                }
                result_idx++;
            }
            summary->total_tests += count;
        } else {
            /* 单结果测试 */
            TestResult *result = &summary->results[result_idx];
            
            /* 初始化结果 */
            result->id = test->id;
            result->name = test->name;
            result->p_value = 0.0;
            result->q_value = 0.0;
            result->stat_value = 0.0;
            result->status = TEST_ERROR;
            result->message = NULL;
            
            /* 运行测试 */
            TestStatus status = test->func(data, n, result, config);
            result->status = status;
            
            switch (status) {
                case TEST_SUCCESS:
                    summary->passed_tests++;
                    break;
                case TEST_FAILURE:
                    summary->failed_tests++;
                    break;
                case TEST_ERROR:
                    summary->error_tests++;
                    break;
            }
            
            result_idx++;
            summary->total_tests++;
        }
    }
    
    summary->results_count = result_idx;
    return 0;
}

int gmrtest_run_file(const char *filename,
                     const char **test_ids, int test_count,
                     TestSummary *summary, const TestConfig *config)
{
    BitData *data = NULL;
    int n;
    int ret;
    
    if (filename == NULL || summary == NULL) {
        return -1;
    }
    
    n = read_bits_from_file_alloc(filename, &data);
    if (n <= 0) {
        return -1;
    }
    
    if (test_ids == NULL || test_count == 0) {
        ret = gmrtest_run_all(data, n, summary, config);
    } else {
        ret = gmrtest_run_multiple(test_ids, test_count, data, n, summary, config);
    }
    
    free(data);
    return (ret == 0) ? n : -1;
}

/* ============================================================================
 * 信息函数
 * ============================================================================ */

void gmrtest_list_tests(FILE *fp)
{
    int i;
    
    fprintf(fp, "可用测试列表 (共 %d 项):\n", TEST_COUNT);
    fprintf(fp, "%-25s %-20s %10s\n", "ID", "名称", "最小长度");
    fprintf(fp, "----------------------------------------------------------\n");
    
    for (i = 0; i < TEST_COUNT; i++) {
        fprintf(fp, "%-25s %-20s %10d\n", 
                all_tests[i].id, 
                all_tests[i].name, 
                all_tests[i].min_length);
    }
}

void gmrtest_print_help(FILE *fp)
{
    fprintf(fp, "gmrtest - 国密随机数检测工具\n");
    fprintf(fp, "使用 -h 选项查看详细帮助\n");
}
