/**
 * @file main.c
 * @brief gmrtest 命令行入口
 * @author gmrtest
 * @date 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "gmrtest.h"

/* ============================================================================
 * 命令行选项定义
 * ============================================================================ */

static struct option long_options[] = {
    {"file",     required_argument, 0, 'f'},
    {"output",   required_argument, 0, 'o'},
    {"tests",    required_argument, 0, 't'},
    {"alpha",    required_argument, 0, 'a'},
    {"format",   required_argument, 0, 'F'},
    {"list",     no_argument,       0, 'l'},
    {"help",     no_argument,       0, 'h'},
    {"version",  no_argument,       0, 'v'},
    {"quiet",    no_argument,       0, 'q'},
    {0, 0, 0, 0}
};

static const char *short_options = "f:o:t:a:F:lhvq";

/* ============================================================================
 * 输出格式枚举
 * ============================================================================ */

typedef enum {
    FORMAT_TEXT = 0,
    FORMAT_JSON = 1,
    FORMAT_CSV  = 2
} OutputFormat;

/* ============================================================================
 * 命令行参数结构
 * ============================================================================ */

typedef struct {
    const char *input_file;
    const char *output_file;
    const char *test_list;
    double alpha;
    OutputFormat format;
    int list_tests;
    int show_help;
    int show_version;
    int quiet;
} CliOptions;

/* ============================================================================
 * 函数声明
 * ============================================================================ */

static void print_banner(FILE *fp);
static void print_usage(const char *prog_name);
static int parse_options(int argc, char *argv[], CliOptions *opts);
static int run_tests(const CliOptions *opts);

/* ============================================================================
 * 主函数
 * ============================================================================ */

int main(int argc, char *argv[])
{
    CliOptions opts = {
        .input_file = NULL,
        .output_file = NULL,
        .test_list = NULL,
        .alpha = DEFAULT_ALPHA,
        .format = FORMAT_TEXT,
        .list_tests = 0,
        .show_help = 0,
        .show_version = 0,
        .quiet = 0
    };

    /* 解析命令行参数 */
    if (parse_options(argc, argv, &opts) != 0) {
        return EXIT_FAILURE;
    }

    /* 显示帮助 */
    if (opts.show_help) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    /* 显示版本 */
    if (opts.show_version) {
        print_banner(stdout);
        return EXIT_SUCCESS;
    }

    /* 列出所有测试 */
    if (opts.list_tests) {
        gmrtest_list_tests(stdout);
        return EXIT_SUCCESS;
    }

    /* 检查必需参数 */
    if (opts.input_file == NULL) {
        fprintf(stderr, "错误: 未指定输入文件\n");
        fprintf(stderr, "使用 '%s -h' 查看帮助信息\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* 运行测试 */
    return run_tests(&opts);
}

/* ============================================================================
 * 函数实现
 * ============================================================================ */

/**
 * @brief 打印程序标识
 */
static void print_banner(FILE *fp)
{
    fprintf(fp, "gmrtest - 国密随机数检测工具 v%s\n", gmrtest_version());
    fprintf(fp, "Copyright (C) 2024. 实现国密规范要求的随机性统计检测算法\n");
}

/**
 * @brief 打印使用说明
 */
static void print_usage(const char *prog_name)
{
    print_banner(stdout);
    printf("\n");
    printf("用法: %s [选项] -f <输入文件>\n", prog_name);
    printf("\n");
    printf("选项:\n");
    printf("  -f, --file <文件>      输入文件路径（二进制格式）\n");
    printf("  -o, --output <文件>    输出文件路径（默认输出到标准输出）\n");
    printf("  -t, --tests <列表>     指定要运行的测试（逗号分隔，默认全部）\n");
    printf("  -a, --alpha <值>       显著性水平（默认 %.2f）\n", DEFAULT_ALPHA);
    printf("  -F, --format <格式>    输出格式: text, json, csv（默认 text）\n");
    printf("  -l, --list             列出所有可用测试\n");
    printf("  -q, --quiet            静默模式，只输出结果\n");
    printf("  -h, --help             显示帮助信息\n");
    printf("  -v, --version          显示版本信息\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s -f random.bin                    # 运行所有测试\n", prog_name);
    printf("  %s -f random.bin -o result.json     # 输出到文件\n", prog_name);
    printf("  %s -f random.bin -t frequency,runs  # 运行指定测试\n", prog_name);
    printf("  %s -f random.bin -F json            # JSON格式输出\n", prog_name);
    printf("  %s -l                               # 列出所有测试\n", prog_name);
    printf("\n");
    printf("可用测试ID:\n");
    printf("  frequency, block_frequency, cumulative_sums, runs,\n");
    printf("  longest_run_ones, rank, dft, non_overlapping_template,\n");
    printf("  overlapping_template, universal, approximate_entropy,\n");
    printf("  random_excursions, random_excursions_variant, serial,\n");
    printf("  linear_complexity, binary_derivate, self_correlation,\n");
    printf("  poker, runs_distribution\n");
}

/**
 * @brief 解析命令行选项
 */
static int parse_options(int argc, char *argv[], CliOptions *opts)
{
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (opt) {
            case 'f':
                opts->input_file = optarg;
                break;
            case 'o':
                opts->output_file = optarg;
                break;
            case 't':
                opts->test_list = optarg;
                break;
            case 'a':
                opts->alpha = atof(optarg);
                if (opts->alpha <= 0.0 || opts->alpha >= 1.0) {
                    fprintf(stderr, "错误: 显著性水平必须在 (0, 1) 范围内\n");
                    return -1;
                }
                break;
            case 'F':
                if (strcmp(optarg, "json") == 0) {
                    opts->format = FORMAT_JSON;
                } else if (strcmp(optarg, "csv") == 0) {
                    opts->format = FORMAT_CSV;
                } else if (strcmp(optarg, "text") == 0) {
                    opts->format = FORMAT_TEXT;
                } else {
                    fprintf(stderr, "错误: 未知的输出格式 '%s'\n", optarg);
                    return -1;
                }
                break;
            case 'l':
                opts->list_tests = 1;
                break;
            case 'h':
                opts->show_help = 1;
                break;
            case 'v':
                opts->show_version = 1;
                break;
            case 'q':
                opts->quiet = 1;
                break;
            case '?':
            default:
                return -1;
        }
    }

    return 0;
}

/**
 * @brief 运行测试
 */
static int run_tests(const CliOptions *opts)
{
    BitData *data = NULL;
    TestSummary summary;
    TestConfig config;
    FILE *out_fp = stdout;
    int ret = EXIT_SUCCESS;

    /* 初始化配置 */
    gmrtest_default_config(&config);
    config.alpha = opts->alpha;

    /* 读取输入文件 */
    if (!opts->quiet) {
        fprintf(stderr, "读取文件: %s\n", opts->input_file);
    }

    int n = read_bits_from_file_alloc(opts->input_file, &data);
    if (n <= 0) {
        fprintf(stderr, "错误: 无法读取文件 '%s'\n", opts->input_file);
        return EXIT_FAILURE;
    }

    if (!opts->quiet) {
        fprintf(stderr, "序列长度: %d 比特\n", n);
    }

    /* 初始化结果汇总 */
    if (summary_init(&summary, TEST_COUNT) != 0) {
        fprintf(stderr, "错误: 内存分配失败\n");
        free(data);
        return EXIT_FAILURE;
    }

    /* 运行测试 */
    if (opts->test_list == NULL) {
        /* 运行所有测试 */
        if (!opts->quiet) {
            fprintf(stderr, "运行所有测试...\n");
        }
        gmrtest_run_all(data, n, &summary, &config);
    } else {
        /* 运行指定测试 */
        /* TODO: 解析测试列表并运行指定测试 */
        if (!opts->quiet) {
            fprintf(stderr, "运行指定测试: %s\n", opts->test_list);
        }
        /* 暂时运行所有测试 */
        gmrtest_run_all(data, n, &summary, &config);
    }

    /* 打开输出文件 */
    if (opts->output_file != NULL) {
        out_fp = fopen(opts->output_file, "w");
        if (out_fp == NULL) {
            fprintf(stderr, "错误: 无法打开输出文件 '%s'\n", opts->output_file);
            ret = EXIT_FAILURE;
            goto cleanup;
        }
    }

    /* 输出结果 */
    switch (opts->format) {
        case FORMAT_JSON:
            output_json(out_fp, &summary, opts->input_file);
            break;
        case FORMAT_CSV:
            output_csv(out_fp, &summary);
            break;
        case FORMAT_TEXT:
        default:
            output_text(out_fp, &summary);
            break;
    }

    /* 关闭输出文件 */
    if (opts->output_file != NULL) {
        fclose(out_fp);
    }

    /* 输出摘要 */
    if (!opts->quiet) {
        fprintf(stderr, "\n测试完成: %d 通过, %d 失败, %d 错误\n",
                summary.passed_tests, summary.failed_tests, summary.error_tests);
    }

    /* 判断整体结果 */
    if (summary.failed_tests > 0 || summary.error_tests > 0) {
        ret = EXIT_FAILURE;
    }

cleanup:
    summary_free(&summary);
    free(data);

    return ret;
}
