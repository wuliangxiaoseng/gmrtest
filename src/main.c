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
    {"config",   required_argument, 0, 'c'},
    {"gm",       no_argument,       0, 'g'},
    {"nist",     no_argument,       0, 'n'},
    {"list",     no_argument,       0, 'l'},
    {"help",     no_argument,       0, 'h'},
    {"version",  no_argument,       0, 'v'},
    {"quiet",    no_argument,       0, 'q'},
    {0, 0, 0, 0}
};

static const char *short_options = "f:o:t:a:F:c:gnlhvq";

/* ============================================================================
 * 输出格式枚举
 * ============================================================================ */

typedef enum {
    FORMAT_TEXT = 0,
    FORMAT_JSON = 1,
    FORMAT_CSV  = 2
} OutputFormat;

/* ============================================================================
 * 测试预设枚举
 * ============================================================================ */

typedef enum {
    PRESET_NONE = 0,
    PRESET_GM,      /* 国密标准测试 */
    PRESET_NIST     /* NIST标准测试 */
} TestPreset;

/* ============================================================================
 * 命令行参数结构
 * ============================================================================ */

typedef struct {
    const char *input_file;
    const char *output_file;
    const char *config_file;
    const char *test_list;
    double alpha;
    OutputFormat format;
    TestPreset preset;
    int list_tests;
    int show_help;
    int show_version;
    int quiet;
    
    /* 测试项启用状态 */
    int test_enabled[50];
} CliOptions;

/* ============================================================================
 * 测试预设定义
 * ============================================================================ */

/* ============================================================================
 * 测试项配置定义（用于配置文件）
 * ============================================================================ */

/* 测试项配置项（用于配置文件） */
typedef struct {
    const char *config_key;   /* 配置文件中的键，如 "poker-m=4" */
    const char *algorithm;     /* 算法ID */
    const char *param;        /* 参数值，如 "m=4" */
    const char *name;         /* 显示名称 */
} TestConfigEntry;

static const TestConfigEntry config_entries[] = {
    /* 基础测试 */
    {"frequency", "frequency", NULL, "单比特频数检测"},
    {"block_frequency-m=10000", "block_frequency", "m=10000", "块内频数检测 (m=10000)"},
    
    /* 累加和检测 */
    {"cumulative_sums-forward", "cumulative_sums", "forward", "累加和检测（前向）"},
    {"cumulative_sums-backward", "cumulative_sums", "backward", "累加和检测（后向）"},
    
    /* 游程测试 */
    {"runs", "runs", NULL, "游程总数检测"},
    {"runs_distribution", "runs_distribution", NULL, "游程分布检测"},
    {"longest_run-m=10000,ones", "longest_run", "m=10000,ones", "块内最大1游程检测 (m=10000)"},
    {"longest_run-m=10000,zeros", "longest_run", "m=10000,zeros", "块内最大0游程检测 (m=10000)"},
    
    /* 扑克检测 */
    {"poker-m=4", "poker", "m=4", "扑克检测 (m=4)"},
    {"poker-m=8", "poker", "m=8", "扑克检测 (m=8)"},
    
    /* 重叠子序列检测 */
    {"serial-m=3", "serial", "m=3", "重叠子序列检测 (m=3)"},
    {"serial-m=5", "serial", "m=5", "重叠子序列检测 (m=5)"},
    
    /* 矩阵/变换测试 */
    {"rank", "rank", NULL, "矩阵秩检测"},
    {"dft", "dft", NULL, "离散傅立叶检测"},
    
    /* 模板匹配测试 */
    {"non_overlapping_template", "non_overlapping_template", NULL, "非重叠模板匹配"},
    {"overlapping_template", "overlapping_template", NULL, "重叠模板匹配"},
    
    /* 熵/复杂度测试 */
    {"universal", "universal", NULL, "Maurer通用统计检测"},
    {"approximate_entropy-m=2", "approximate_entropy", "m=2", "近似熵检测 (m=2)"},
    {"approximate_entropy-m=5", "approximate_entropy", "m=5", "近似熵检测 (m=5)"},
    {"linear_complexity-m=500", "linear_complexity", "m=500", "线性复杂度检测 (m=500)"},
    {"linear_complexity-m=1000", "linear_complexity", "m=1000", "线性复杂度检测 (m=1000)"},
    
    /* 随机游程测试 */
    {"random_excursions", "random_excursions", NULL, "随机游程检测"},
    {"random_excursions_variant", "random_excursions_variant", NULL, "随机游程变体检测"},
    
    /* 其他测试 */
    {"binary_derivative-k=3", "binary_derivative", "k=3", "二元推导检测 (k=3)"},
    {"binary_derivative-k=7", "binary_derivative", "k=7", "二元推导检测 (k=7)"},
    {"autocorrelation-d=1", "autocorrelation", "d=1", "自相关检测 (d=1)"},
    {"autocorrelation-d=2", "autocorrelation", "d=2", "自相关检测 (d=2)"},
    {"autocorrelation-d=8", "autocorrelation", "d=8", "自相关检测 (d=8)"},
    {"autocorrelation-d=16", "autocorrelation", "d=16", "自相关检测 (d=16)"},
};

#define CONFIG_ENTRY_COUNT ((int)(sizeof(config_entries) / sizeof(config_entries[0])))

/* 测试项启用状态（按配置项索引） */
typedef struct {
    int enabled;              /* 是否启用 */
    const char *param;        /* 参数值 */
} TestConfigState;

/* 国密标准测试（27 项独立检测，对应配置项键名） */
static const char *gm_tests[] = {
    "frequency",
    "block_frequency-m=10000",
    "poker-m=4",
    "poker-m=8",
    "serial-m=3",
    "serial-m=5",
    "runs",
    "runs_distribution",
    "longest_run-m=10000,ones",
    "longest_run-m=10000,zeros",
    "binary_derivative-k=3",
    "binary_derivative-k=7",
    "autocorrelation-d=1",
    "autocorrelation-d=2",
    "autocorrelation-d=8",
    "autocorrelation-d=16",
    "rank",
    "cumulative_sums-forward",
    "cumulative_sums-backward",
    "approximate_entropy-m=2",
    "approximate_entropy-m=5",
    "linear_complexity-m=500",
    "linear_complexity-m=1000",
    "universal",
    "dft",
    NULL
};

/* NIST标准测试（21项独立检测） */
static const char *nist_tests[] = {
    "frequency",
    "block_frequency-m=10000",
    "cumulative_sums-forward",
    "cumulative_sums-backward",
    "runs",
    "longest_run-m=10000,ones",
    "rank",
    "dft",
    "non_overlapping_template",
    "overlapping_template",
    "universal",
    "approximate_entropy-m=2",
    "approximate_entropy-m=5",
    "random_excursions",
    "random_excursions_variant",
    "serial-m=3",
    "serial-m=5",
    "linear_complexity-m=500",
    "linear_complexity-m=1000",
    "poker-m=4",
    "poker-m=8",
    "runs_distribution",
    "binary_derivative-k=3",
    "binary_derivative-k=7",
    "autocorrelation-d=1",
    "autocorrelation-d=2",
    "autocorrelation-d=8",
    "autocorrelation-d=16",
    NULL
};

/* ============================================================================
 * 函数声明
 * ============================================================================ */

static void print_banner(FILE *fp);
static void print_usage(const char *prog_name);
static void print_test_list(FILE *fp);
static int parse_options(int argc, char *argv[], CliOptions *opts);
static int load_config_file(const char *filename, CliOptions *opts);
static int run_tests(const CliOptions *opts);

/* ============================================================================
 * 主函数
 * ============================================================================ */

int main(int argc, char *argv[])
{
    CliOptions opts = {
        .input_file = NULL,
        .output_file = NULL,
        .config_file = NULL,
        .test_list = NULL,
        .alpha = DEFAULT_ALPHA,
        .format = FORMAT_TEXT,
        .preset = PRESET_NONE,
        .list_tests = 0,
        .show_help = 0,
        .show_version = 0,
        .quiet = 0
    };

    /* 解析命令行参数（先处理 -c/--config） */
    if (parse_options(argc, argv, &opts) != 0) {
        return EXIT_FAILURE;
    }

    /* 如果指定了配置文件，加载配置 */
    if (opts.config_file != NULL) {
        if (!opts.quiet) {
            fprintf(stderr, "加载配置文件: %s\n", opts.config_file);
        }
        if (load_config_file(opts.config_file, &opts) != 0) {
            fprintf(stderr, "警告: 无法加载配置文件 '%s'\n", opts.config_file);
        }
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
        print_test_list(stdout);
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
    printf("  -c, --config <文件>    配置文件路径\n");
    printf("  -g, --gm               运行国密标准测试（10项）\n");
    printf("  -n, --nist             运行NIST标准测试（21项）\n");
    printf("  -a, --alpha <值>       显著性水平（默认 %.2f）\n", DEFAULT_ALPHA);
    printf("  -F, --format <格式>    输出格式: text, json, csv（默认 text）\n");
    printf("  -l, --list             列出所有可用测试\n");
    printf("  -q, --quiet            静默模式，只输出结果\n");
    printf("  -h, --help             显示帮助信息\n");
    printf("  -v, --version          显示版本信息\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s -f random.bin                    # 运行所有测试\n", prog_name);
    printf("  %s -f random.bin -g                 # 运行国密标准测试\n", prog_name);
    printf("  %s -f random.bin -n                 # 运行NIST标准测试\n", prog_name);
    printf("  %s -c config.ini                    # 使用配置文件\n", prog_name);
    printf("  %s -c config.ini -o result.json     # 配置文件+输出文件\n", prog_name);
    printf("\n");
    printf("配置文件格式（INI格式）:\n");
    printf("  # gmrtest 配置文件\n");
    printf("  [gmrtest]\n");
    printf("  input_file = test/random.bin\n");
    printf("  format = text\n");
    printf("  alpha = 0.01\n");
    printf("  \n");
    printf("  # 测试项配置 (0=禁用, 1=启用)\n");
    printf("  frequency = 1\n");
    printf("  block_frequency = 1\n");
    printf("  runs = 1\n");
    printf("  poker_m4 = 1\n");
    printf("  poker_m8 = 1\n");
    printf("  serial_m3_p1 = 1\n");
    printf("  serial_m3_p2 = 1\n");
    printf("  # ... 其他测试项\n");
    printf("\n");
    printf("支持的所有测试项（共24项）:\n");
    printf("  基础测试:\n");
    printf("    frequency              单比特频数检测\n");
    printf("    block_frequency       块内频数检测\n");
    printf("    cumulative_sums       累加和检测\n");
    printf("  \n");
    printf("  游程测试:\n");
    printf("    runs                  游程总数检测\n");
    printf("    runs_distribution     游程分布检测\n");
    printf("    longest_run_ones      最长1游程检测\n");
    printf("  \n");
    printf("  扑克检测（不同参数作为独立测试项）:\n");
    printf("    poker_m4              扑克检测 (m=4)\n");
    printf("    poker_m8              扑克检测 (m=8)\n");
    printf("  \n");
    printf("  重叠子序列检测（不同参数作为独立测试项）:\n");
    printf("    serial_m3_p1          重叠子序列检测 (m=3, P1)\n");
    printf("    serial_m3_p2          重叠子序列检测 (m=3, P2)\n");
    printf("    serial_m5_p1          重叠子序列检测 (m=5, P1)\n");
    printf("    serial_m5_p2          重叠子序列检测 (m=5, P2)\n");
    printf("  \n");
    printf("  矩阵/变换测试:\n");
    printf("    rank                  矩阵秩检测\n");
    printf("    dft                   离散傅立叶检测\n");
    printf("  \n");
    printf("  模板匹配测试:\n");
    printf("    non_overlapping_template  非重叠模板匹配\n");
    printf("    overlapping_template  重叠模板匹配\n");
    printf("  \n");
    printf("  熵/复杂度测试:\n");
    printf("    universal             通用统计检测\n");
    printf("    approximate_entropy  近似熵检测\n");
    printf("    linear_complexity     线性复杂度检测\n");
    printf("  \n");
    printf("  随机游程测试:\n");
    printf("    random_excursions     随机游程检测\n");
    printf("    random_excursions_variant 随机游程变体检测\n");
    printf("  \n");
    printf("  其他测试:\n");
    printf("    binary_derivate        二元推导检测\n");
    printf("    self_correlation      自相关检测\n");
}

/**
 * @brief 列出所有可用测试项
 */
static void print_test_list(FILE *fp)
{
    int i;
    
    fprintf(fp, "gmrtest - 支持的测试项清单\n");
    fprintf(fp, "============================\n\n");
    fprintf(fp, "共 %d 项独立检测项\n\n", CONFIG_ENTRY_COUNT);
    
    for (i = 0; i < CONFIG_ENTRY_COUNT; i++) {
        fprintf(fp, "  %-35s %s\n", config_entries[i].config_key, config_entries[i].name);
    }
    
    fprintf(fp, "\n");
    fprintf(fp, "配置文件格式示例:\n");
    fprintf(fp, "  [gmrtest]\n");
    fprintf(fp, "  input_file = test/random.bin\n");
    fprintf(fp, "  format = text\n");
    fprintf(fp, "  alpha = 0.01\n");
    fprintf(fp, "  \n");
    fprintf(fp, "  # 测试项配置 (0=禁用, 1=启用)\n");
    fprintf(fp, "  frequency = 1\n");
    fprintf(fp, "  block_frequency-m=10000 = 1\n");
    fprintf(fp, "  poker-m=4 = 1\n");
    fprintf(fp, "  poker-m=8 = 1\n");
    fprintf(fp, "  serial-m=3 = 1\n");
    fprintf(fp, "  serial-m=5 = 1\n");
    fprintf(fp, "  # ...\n");
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
            case 'c':
                opts->config_file = optarg;
                break;
            case 't':
                opts->test_list = optarg;
                break;
            case 'g':
                opts->preset = PRESET_GM;
                break;
            case 'n':
                opts->preset = PRESET_NIST;
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
 * @brief 加载配置文件（INI格式）
 */
static int load_config_file(const char *filename, CliOptions *opts)
{
    FILE *fp;
    char line[256];
    char key[64], value[128];
    int in_section = 0;
    int i;

    if (filename == NULL || opts == NULL) {
        return -1;
    }

    /* 初始化所有测试为禁用 */
    for (i = 0; i < CONFIG_ENTRY_COUNT; i++) {
        opts->test_enabled[i] = 0;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        /* 去除换行符 */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }

        /* 跳过空行和注释 */
        if (line[0] == '#' || line[0] == ';' || line[0] == '\0') {
            continue;
        }

        /* 去除行内注释（# 和 ;） */
        {
            char *comment = strchr(line, '#');
            if (comment != NULL) {
                *comment = '\0';
            }
            comment = strchr(line, ';');
            if (comment != NULL) {
                *comment = '\0';
            }
        }

        /* 检查节头 */
        if (line[0] == '[') {
            in_section = (strstr(line, "gmrtest") != NULL);
            continue;
        }

        /* 只处理 [gmrtest] 节中的内容 */
        if (!in_section) {
            continue;
        }

        /* 解析 key = value，key可能含有特殊字符如 '-' 和 '=' */
        {
            /* 找最后一个等号（配置值的分隔符） */
            char *last_equals = strrchr(line, '=');
            if (last_equals != NULL && last_equals > line) {
                /* 提取 key（去除末尾空格） */
                size_t key_len = last_equals - line;
                if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
                strncpy(key, line, key_len);
                key[key_len] = '\0';
                char *key_end = key + strlen(key) - 1;
                while (key_end >= key && (*key_end == ' ' || *key_end == '\t')) {
                    *key_end-- = '\0';
                }
                
                /* 提取 value（去除首尾空格） */
                char *val_ptr = last_equals + 1;
                while (*val_ptr == ' ' || *val_ptr == '\t') val_ptr++;
                char *val_end_ptr = val_ptr + strlen(val_ptr) - 1;
                while (val_end_ptr > val_ptr && (*val_end_ptr == ' ' || *val_end_ptr == '\t' || *val_end_ptr == '\r' || *val_end_ptr == '\n')) {
                    *val_end_ptr-- = '\0';
                }
                strncpy(value, val_ptr, sizeof(value) - 1);
                value[sizeof(value) - 1] = '\0';
            } else {
                continue;
            }
        }
        
        if (key[0] != '\0' && value[0] != '\0') {
            char *key_start = key;
            char *val_start = value;

            /* 解析全局配置项 */
            if (strcmp(key_start, "input_file") == 0) {
                opts->input_file = strdup(val_start);
            } else if (strcmp(key_start, "output_file") == 0) {
                opts->output_file = strdup(val_start);
            } else if (strcmp(key_start, "alpha") == 0) {
                opts->alpha = atof(val_start);
            } else if (strcmp(key_start, "format") == 0) {
                if (strcmp(val_start, "json") == 0) {
                    opts->format = FORMAT_JSON;
                } else if (strcmp(val_start, "csv") == 0) {
                    opts->format = FORMAT_CSV;
                } else if (strcmp(val_start, "text") == 0) {
                    opts->format = FORMAT_TEXT;
                }
            } else if (strcmp(key_start, "quiet") == 0) {
                if (strcmp(val_start, "true") == 0 || strcmp(val_start, "1") == 0) {
                    opts->quiet = 1;
                }
            } else {
                /* 检查是否为测试项配置 */
                for (i = 0; i < CONFIG_ENTRY_COUNT; i++) {
                    if (strcmp(key_start, config_entries[i].config_key) == 0) {
                        opts->test_enabled[i] = atoi(val_start);
                        break;
                    }
                }
            }
        }
    }

    fclose(fp);
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
    const char **test_ids = NULL;
    int test_count = 0;
    const char *enabled_tests[50];
    int i;

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

    /* 确定要运行的测试 */
    /* 优先使用配置文件中的测试项设置 */
    test_count = 0;
    for (i = 0; i < CONFIG_ENTRY_COUNT; i++) {
        if (opts->test_enabled[i] == 1) {
            enabled_tests[test_count++] = config_entries[i].config_key;
        }
    }

    if (test_count > 0) {
        /* 使用配置文件中的测试项 */
        test_ids = enabled_tests;
        if (!opts->quiet) {
            fprintf(stderr, "运行配置文件中的 %d 项测试...\n", test_count);
        }
    } else if (opts->preset == PRESET_GM) {
        test_ids = gm_tests;
        if (!opts->quiet) {
            fprintf(stderr, "运行国密标准测试（27项）...\n");
        }
    } else if (opts->preset == PRESET_NIST) {
        test_ids = nist_tests;
        if (!opts->quiet) {
            fprintf(stderr, "运行NIST标准测试（28项）...\n");
        }
    } else {
        test_ids = NULL;
        if (!opts->quiet) {
            fprintf(stderr, "运行所有测试...\n");
        }
    }

    /* 运行测试 */
    if (test_ids != NULL && test_count > 0) {
        gmrtest_run_multiple(test_ids, test_count, data, n, &summary, &config);
    } else if (test_ids != NULL) {
        /* 计算测试数量 */
        while (test_ids[test_count] != NULL) {
            test_count++;
        }
        gmrtest_run_multiple(test_ids, test_count, data, n, &summary, &config);
    } else {
        /* 运行所有测试 */
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
