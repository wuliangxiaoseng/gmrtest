/**
 * @file gmrtest.c
 * @brief gmrtest API 实现
 * @author gmrtest
 * @date 2024
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gmrtest.h"
#include "tests.h"
#include "thread_pool.h"

/* ============================================================================
 * 并行执行内部数据结构
 * ============================================================================ */

typedef struct {
    const TestItem *test;
    TestResult      results[4];
    int             count;
    int             valid;
} GmrSlot;

typedef struct {
    GmrSlot          *slots;
    const BitData    *data;
    int               n;
    const TestConfig *config;
} GmrRunCtx;

static double gmr_elapsed_ms(const struct timespec *t0, const struct timespec *t1)
{
    return (double)(t1->tv_sec - t0->tv_sec) * 1000.0
         + (double)(t1->tv_nsec - t0->tv_nsec) / 1e6;
}

static void gmr_run_one_slot(void *arg, int task_index)
{
    GmrRunCtx *ctx = (GmrRunCtx *)arg;
    GmrSlot   *slot = &ctx->slots[task_index];
    const TestItem *test = slot->test;
    struct timespec t0, t1;
    double ms;
    int k;

    if (!slot->valid || test == NULL) return;

    if (test->multi_func != NULL) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        int count = test->multi_func(ctx->data, ctx->n, slot->results, 4, ctx->config);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        slot->count = count > 0 ? count : 0;
        ms = gmr_elapsed_ms(&t0, &t1);
        for (k = 0; k < slot->count; k++) {
            slot->results[k].elapsed_ms = ms;
        }
    } else {
        TestResult *r = &slot->results[0];
        r->id = test->id;
        r->name = test->name;
        r->p_value = 0.0;
        r->q_value = 0.0;
        r->stat_value = 0.0;
        r->elapsed_ms = 0.0;
        r->status = TEST_ERROR;
        r->message = NULL;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        TestStatus st = test->func(ctx->data, ctx->n, r, ctx->config);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        r->elapsed_ms = gmr_elapsed_ms(&t0, &t1);
        r->status = st;
        slot->count = 1;
    }
}

/* ============================================================================
 * 测试项定义
 * ============================================================================ */

static TestItem all_tests[TEST_COUNT] = {
    /* ===== 国密 GM/T 0005-2021 要求的 27 项独立检测项 ===== */

    /* 1. 单比特频数检测 */
    {TEST_ID_FREQUENCY_STR, "单比特频数检测", "检测序列中0和1的比例",
     test_frequency, NULL, MIN_LENGTH_FREQUENCY},

    /* 2. 块内频数检测 m=10000 */
    {TEST_ID_BLOCK_FREQUENCY_M10000_STR, "块内频数检测(m=10000)", "块内1的比例分布",
     test_block_frequency, NULL, MIN_LENGTH_BLOCK_FREQUENCY},

    /* 3-4. 扑克检测 m=4 / m=8 (multi_func 提供 m=4,m=8 两个结果) */
    {TEST_ID_POKER_STR, "扑克检测", "块模式分布",
     NULL, test_poker, MIN_LENGTH_POKER},

    /* 5-8. 重叠子序列检测 m=3 / m=5 (multi_func 提供 4 个结果) */
    {TEST_ID_SERIAL_STR, "重叠子序列检测", "连续模式频率分布",
     NULL, test_serial, MIN_LENGTH_SERIAL},

    /* 9. 游程总数检测 */
    {TEST_ID_RUNS_STR, "游程总数检测", "游程数量分布",
     test_runs, NULL, MIN_LENGTH_RUNS},

    /* 10. 游程分布检测 */
    {TEST_ID_RUNS_DISTRIBUTION_STR, "游程分布检测", "游程长度分布",
     test_runs_distribution, NULL, MIN_LENGTH_RUNS_DISTRIBUTION},

    /* 11. 块内最大1游程检测 m=10000 */
    {TEST_ID_LONGEST_RUN_M10000_ONES_STR, "块内最大1游程检测(m=10000)", "最长连续1的长度",
     test_longest_run_ones, NULL, MIN_LENGTH_LONGEST_RUN_ONES},

    /* 12. 块内最大0游程检测 m=10000 */
    {TEST_ID_LONGEST_RUN_M10000_ZEROS_STR, "块内最大0游程检测(m=10000)", "最长连续0的长度",
     test_longest_run_zeros, NULL, MIN_LENGTH_LONGEST_RUN_ONES},

    /* 13. 二元推导检测 k=3 */
    {TEST_ID_BINARY_DERIVATIVE_K3_STR, "二元推导检测(k=3)", "k=3次异或推导序列",
     test_binary_derivative_k3, NULL, MIN_LENGTH_BINARY_DERIVATE},

    /* 14. 二元推导检测 k=7 */
    {TEST_ID_BINARY_DERIVATIVE_K7_STR, "二元推导检测(k=7)", "k=7次异或推导序列",
     test_binary_derivative_k7, NULL, MIN_LENGTH_BINARY_DERIVATE},

    /* 15-18. 自相关检测 d=1/2/8/16 */
    {TEST_ID_AUTOCORRELATION_D1_STR, "自相关检测(d=1)", "自相关性",
     test_self_correlation_d1, NULL, MIN_LENGTH_SELF_CORRELATION},
    {TEST_ID_AUTOCORRELATION_D2_STR, "自相关检测(d=2)", "自相关性",
     test_self_correlation_d2, NULL, MIN_LENGTH_SELF_CORRELATION},
    {TEST_ID_AUTOCORRELATION_D8_STR, "自相关检测(d=8)", "自相关性",
     test_self_correlation_d8, NULL, MIN_LENGTH_SELF_CORRELATION},
    {TEST_ID_AUTOCORRELATION_D16_STR, "自相关检测(d=16)", "自相关性",
     test_self_correlation_d16, NULL, MIN_LENGTH_SELF_CORRELATION},

    /* 19. 矩阵秩检测 */
    {TEST_ID_RANK_STR, "矩阵秩检测", "32x32矩阵秩分布",
     test_rank, NULL, MIN_LENGTH_RANK},

    /* 20-21. 累加和检测（前向/后向） */
    {TEST_ID_CUMULATIVE_SUMS_FORWARD_STR, "累加和检测(前向)", "累积和随机游走(前向)",
     test_cumulative_sums_forward, NULL, MIN_LENGTH_CUMULATIVE_SUMS},
    {TEST_ID_CUMULATIVE_SUMS_BACKWARD_STR, "累加和检测(后向)", "累积和随机游走(后向)",
     test_cumulative_sums_backward, NULL, MIN_LENGTH_CUMULATIVE_SUMS},

    /* 22-23. 近似熵检测 m=2 / m=5 */
    {TEST_ID_APPROXIMATE_ENTROPY_M2_STR, "近似熵检测(m=2)", "近似熵复杂度",
     test_approximate_entropy_m2, NULL, MIN_LENGTH_APPROXIMATE_ENTROPY},
    {TEST_ID_APPROXIMATE_ENTROPY_M5_STR, "近似熵检测(m=5)", "近似熵复杂度",
     test_approximate_entropy_m5, NULL, MIN_LENGTH_APPROXIMATE_ENTROPY},

    /* 24-25. 线性复杂度检测 m=500 / m=1000 */
    {TEST_ID_LINEAR_COMPLEXITY_M500_STR, "线性复杂度检测(m=500)", "LFSR最短长度分布",
     test_linear_complexity_m500, NULL, MIN_LENGTH_LINEAR_COMPLEXITY},
    {TEST_ID_LINEAR_COMPLEXITY_M1000_STR, "线性复杂度检测(m=1000)", "LFSR最短长度分布",
     test_linear_complexity_m1000, NULL, MIN_LENGTH_LINEAR_COMPLEXITY},

    /* 26. Maurer 通用统计检测 (L=7,Q=1280) */
    {TEST_ID_UNIVERSAL_STR, "Maurer通用统计检测(L=7,Q=1280)", "序列压缩性",
     test_universal, NULL, MIN_LENGTH_UNIVERSAL},

    /* 27. 离散傅立叶检测 */
    {TEST_ID_DFT_STR, "离散傅立叶检测", "FFT周期性",
     test_dft, NULL, MIN_LENGTH_DFT},

    /* ===== 以下为 NIST 规范但国密不要求的项（保留为桩） ===== */
    {TEST_ID_NON_OVERLAPPING_TMPL_STR, "非重叠模板匹配", "非重叠模板出现频率",
     test_non_overlapping_template, NULL, MIN_LENGTH_NON_OVERLAPPING},
    {TEST_ID_OVERLAPPING_TMPL_STR, "重叠模板匹配", "重叠模板出现频率",
     test_overlapping_template, NULL, MIN_LENGTH_OVERLAPPING},
    {TEST_ID_RANDOM_EXCURSIONS_STR, "随机游程检测", "随机游走返回次数",
     test_random_excursions, NULL, MIN_LENGTH_RANDOM_EXCURSIONS},
    {TEST_ID_RANDOM_EXCURSIONS_VAR_STR, "随机游程变体检测", "扩展状态访问次数",
     test_random_excursions_variant, NULL, MIN_LENGTH_RANDOM_EXCURSIONS_VAR}
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
    config->cusum_mode = 0;
    config->num_threads = 1;   /* 默认单线程 */
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

    /* ----------------------------------------------------------------
     * 阶段 1：解析每个 test_id，得到（TestItem*, sub_index, sub_index2）
     *         同时构建去重的 TestItem 任务表（用于并行执行）
     * ---------------------------------------------------------------- */
    typedef struct {
        const TestItem *test;
        int sub_index;
        int sub_index2;
        int task_slot;
    } IdEntry;

    IdEntry *entries = (IdEntry *)calloc((size_t)test_count, sizeof(IdEntry));
    GmrSlot *unique_tests = (GmrSlot *)calloc((size_t)test_count, sizeof(GmrSlot));
    if (entries == NULL || unique_tests == NULL) {
        free(entries);
        free(unique_tests);
        return -1;
    }
    int unique_count = 0;

    for (i = 0; i < test_count; i++) {
        const char *test_id = test_ids[i];
        const TestItem *test = NULL;
        int sub_index = -1;
        int sub_index2 = -1;

        /* 仅 poker / serial 仍使用 multi_func + sub_index 派发 */
        if (strncmp(test_id, "poker-m=", 8) == 0) {
            const char *param = test_id + 8;
            test = get_test_by_id(TEST_ID_POKER_STR);
            if (strcmp(param, "4") == 0) {
                sub_index = 0;
            } else if (strcmp(param, "8") == 0) {
                sub_index = 1;
            }
        } else if (strncmp(test_id, "serial-m=", 9) == 0) {
            const char *param = test_id + 9;
            test = get_test_by_id(TEST_ID_SERIAL_STR);
            /* 细粒度 ID：serial-m=3,p1 / serial-m=3,p2 / serial-m=5,p1 / serial-m=5,p2 */
            if (strcmp(param, "3,p1") == 0) {
                sub_index = 0;
            } else if (strcmp(param, "3,p2") == 0) {
                sub_index = 1;
            } else if (strcmp(param, "5,p1") == 0) {
                sub_index = 2;
            } else if (strcmp(param, "5,p2") == 0) {
                sub_index = 3;
            } else if (strcmp(param, "3") == 0) {
                /* GM/T 0005-2021 规范要求 m=3 输出双 P 值 */
                sub_index = 0;
                sub_index2 = 1;
            } else if (strcmp(param, "5") == 0) {
                /* GM/T 0005-2021 规范要求 m=5 输出双 P 值 */
                sub_index = 2;
                sub_index2 = 3;
            }
        } else {
            /* 其余完整 ID 均直接通过 get_test_by_id 命中 */
            test = get_test_by_id(test_id);
        }

        entries[i].test = test;
        entries[i].sub_index = sub_index;
        entries[i].sub_index2 = sub_index2;
        entries[i].task_slot = -1;

        if (test == NULL || n < test->min_length) {
            continue;
        }

        /* 在 unique_tests 中查找/插入 */
        int slot = -1;
        for (int u = 0; u < unique_count; u++) {
            if (unique_tests[u].test == test) {
                slot = u;
                break;
            }
        }
        if (slot < 0) {
            slot = unique_count++;
            unique_tests[slot].test = test;
            unique_tests[slot].count = 0;
            unique_tests[slot].valid = 1;  /* 已通过 min_length 检查，执行 */
        }
        entries[i].task_slot = slot;
    }

    /* ----------------------------------------------------------------
     * 阶段 2：并行执行所有去重后的 TestItem 任务
     * ---------------------------------------------------------------- */
    GmrRunCtx ctx;
    ctx.slots = unique_tests;
    ctx.data = data;
    ctx.n = n;
    ctx.config = config;

    parallel_for(gmr_run_one_slot, &ctx, unique_count,
                 config->num_threads);

    /* ----------------------------------------------------------------
     * 阶段 3：按 test_ids 顺序串行汇总结果到 summary
     * ---------------------------------------------------------------- */
    for (i = 0; i < test_count; i++) {
        const TestItem *test = entries[i].test;
        int sub_index = entries[i].sub_index;
        int sub_index2 = entries[i].sub_index2;
        int slot = entries[i].task_slot;

        if (test == NULL || slot < 0 || !unique_tests[slot].valid) {
            continue;
        }
        if (result_idx >= MAX_RESULTS) break;

        if (test->multi_func != NULL) {
            int count = unique_tests[slot].count;

            if (sub_index >= 0 && sub_index < count) {
                summary->results[result_idx] = unique_tests[slot].results[sub_index];
                switch (unique_tests[slot].results[sub_index].status) {
                    case TEST_SUCCESS: summary->passed_tests++; break;
                    case TEST_FAILURE: summary->failed_tests++; break;
                    case TEST_ERROR:   summary->error_tests++;  break;
                }
                result_idx++;
                summary->total_tests++;

                /* serial 双 P 值 */
                if (sub_index2 >= 0 && sub_index2 < count && result_idx < MAX_RESULTS) {
                    summary->results[result_idx] = unique_tests[slot].results[sub_index2];
                    switch (unique_tests[slot].results[sub_index2].status) {
                        case TEST_SUCCESS: summary->passed_tests++; break;
                        case TEST_FAILURE: summary->failed_tests++; break;
                        case TEST_ERROR:   summary->error_tests++;  break;
                    }
                    result_idx++;
                    summary->total_tests++;
                }
            } else if (sub_index < 0) {
                /* 返回所有结果 */
                for (int j = 0; j < count && result_idx < MAX_RESULTS; j++) {
                    summary->results[result_idx] = unique_tests[slot].results[j];
                    switch (unique_tests[slot].results[j].status) {
                        case TEST_SUCCESS: summary->passed_tests++; break;
                        case TEST_FAILURE: summary->failed_tests++; break;
                        case TEST_ERROR:   summary->error_tests++;  break;
                    }
                    result_idx++;
                }
                summary->total_tests += count;
            }
        } else {
            /* 单结果测试 */
            summary->results[result_idx] = unique_tests[slot].results[0];
            switch (unique_tests[slot].results[0].status) {
                case TEST_SUCCESS: summary->passed_tests++; break;
                case TEST_FAILURE: summary->failed_tests++; break;
                case TEST_ERROR:   summary->error_tests++;  break;
            }
            result_idx++;
            summary->total_tests++;
        }
    }

    free(entries);
    free(unique_tests);

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

    /* ----------------------------------------------------------------
     * 阶段 1：为每个 TestItem 分配独立的临时槽，并行执行
     * ---------------------------------------------------------------- */
    GmrSlot *slots = (GmrSlot *)calloc(TEST_COUNT, sizeof(GmrSlot));
    if (slots == NULL) {
        return -1;
    }
    for (i = 0; i < TEST_COUNT; i++) {
        slots[i].test = &all_tests[i];
        slots[i].count = 0;
        slots[i].valid = (n >= all_tests[i].min_length) ? 1 : 0;
    }

    GmrRunCtx ctx;
    ctx.slots = slots;
    ctx.data = data;
    ctx.n = n;
    ctx.config = config;

    parallel_for(gmr_run_one_slot, &ctx, TEST_COUNT,
                 config->num_threads);

    /* ----------------------------------------------------------------
     * 阶段 2：按注册表顺序串行汇总结果
     * ---------------------------------------------------------------- */
    for (i = 0; i < TEST_COUNT; i++) {
        const TestItem *test = slots[i].test;
        if (!slots[i].valid) continue;

        if (test->multi_func != NULL) {
            int count = slots[i].count;
            for (int j = 0; j < count && result_idx < MAX_RESULTS; j++) {
                summary->results[result_idx] = slots[i].results[j];
                switch (slots[i].results[j].status) {
                    case TEST_SUCCESS: summary->passed_tests++; break;
                    case TEST_FAILURE: summary->failed_tests++; break;
                    case TEST_ERROR:   summary->error_tests++;  break;
                }
                result_idx++;
            }
            summary->total_tests += count;
        } else {
            if (result_idx >= MAX_RESULTS) break;
            summary->results[result_idx] = slots[i].results[0];
            switch (slots[i].results[0].status) {
                case TEST_SUCCESS: summary->passed_tests++; break;
                case TEST_FAILURE: summary->failed_tests++; break;
                case TEST_ERROR:   summary->error_tests++;  break;
            }
            result_idx++;
            summary->total_tests++;
        }
    }

    free(slots);
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
