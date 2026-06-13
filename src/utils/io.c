/**
 * @file io.c
 * @brief 输入输出函数实现
 * @author gmrtest
 * @date 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "utils.h"

/* ============================================================================
 * 位操作函数实现
 * ============================================================================ */

void bit_copy(BitData *dest, const BitData *src, int n)
{
    memcpy(dest, src, BITS_TO_BYTES(n));
}

void bit_sequence_free(BitSequence *seq)
{
    if (seq && seq->owns_data && seq->data) {
        free(seq->data);
        seq->data = NULL;
        seq->length = 0;
        seq->allocated = 0;
    }
}

/* ============================================================================
 * 比特序列处理
 * ============================================================================ */

/**
 * @brief 将字节数组转换为比特序列
 * 根据 BIT_PACKING 模式采用不同的转换策略：
 * - BIT_PACKING=0: 展开每个比特到单独字节
 * - BIT_PACKING=1: 直接复制（位压缩格式）
 */
int bytes_to_bits(const uint8_t *bytes, int byte_len, BitData *bits)
{
    int i, j;
    
    if (bytes == NULL || bits == NULL || byte_len <= 0) {
        return -1;
    }
    
#if BIT_PACKING
    /* 位压缩模式：直接复制 */
    memcpy(bits, bytes, byte_len);
#else
    /* 字节/比特模式：展开每个比特到单独字节 */
    for (i = 0; i < byte_len; i++) {
        j = i * 8;
        bits[j]     = (bytes[i] >> 7) & 0x01;
        bits[j + 1] = (bytes[i] >> 6) & 0x01;
        bits[j + 2] = (bytes[i] >> 5) & 0x01;
        bits[j + 3] = (bytes[i] >> 4) & 0x01;
        bits[j + 4] = (bytes[i] >> 3) & 0x01;
        bits[j + 5] = (bytes[i] >> 2) & 0x01;
        bits[j + 6] = (bytes[i] >> 1) & 0x01;
        bits[j + 7] = bytes[i] & 0x01;
    }
#endif
    
    return byte_len * 8;
}

/**
 * @brief 从文件读取比特序列
 * @param filename 文件名
 * @param bits 输出比特序列缓冲区
 * @param max_bits 最大比特数
 * @return 成功返回读取的比特数，失败返回-1
 */
int read_bits_from_file(const char *filename, BitData *bits, int max_bits)
{
    FILE *fp;
    uint8_t *buffer = NULL;
    int byte_len, bit_len;
    
    if (filename == NULL || bits == NULL || max_bits <= 0) {
        return -1;
    }
    
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return -1;
    }
    
    /* 计算需要读取的字节数（原始文件大小） */
    byte_len = BITS_TO_BYTES(max_bits);
    
#if BIT_PACKING
    /* 位压缩模式：直接读取 */
    byte_len = fread(bits, 1, byte_len, fp);
    fclose(fp);
    
    if (byte_len <= 0) {
        return -1;
    }
    
    return byte_len * 8;
#else
    /* 字节/比特模式：需要先读取到缓冲区，再转换 */
    buffer = (uint8_t *)malloc(byte_len);
    if (buffer == NULL) {
        fclose(fp);
        return -1;
    }
    
    byte_len = fread(buffer, 1, byte_len, fp);
    fclose(fp);
    
    if (byte_len <= 0) {
        free(buffer);
        return -1;
    }
    
    bit_len = bytes_to_bits(buffer, byte_len, bits);
    free(buffer);
    
    return bit_len;
#endif
}

/**
 * @brief 从文件读取比特序列（自动分配内存）
 * @param filename 文件名
 * @param bits 输出比特序列指针（函数内分配）
 * @return 成功返回读取的比特数，失败返回-1
 */
int read_bits_from_file_alloc(const char *filename, BitData **bits)
{
    FILE *fp;
    long file_size;
    int byte_len, bit_len;
    BitData *data = NULL;
    uint8_t *buffer = NULL;
    
    if (filename == NULL || bits == NULL) {
        return -1;
    }
    
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return -1;
    }
    
    /* 获取文件大小 */
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(fp);
        return -1;
    }
    
    byte_len = (int)file_size;
    bit_len = byte_len * 8;
    
#if BIT_PACKING
    /* 位压缩模式：直接分配和读取 */
    data = (BitData *)malloc(byte_len);
    if (data == NULL) {
        fclose(fp);
        return -1;
    }
    
    byte_len = fread(data, 1, byte_len, fp);
    fclose(fp);
    
    if (byte_len <= 0) {
        free(data);
        return -1;
    }
    
    *bits = data;
    return byte_len * 8;
#else
    /* 字节/比特模式：需要转换，分配更大的内存 */
    data = (BitData *)malloc(bit_len);  /* 每个比特一个字节 */
    if (data == NULL) {
        fclose(fp);
        return -1;
    }
    
    buffer = (uint8_t *)malloc(byte_len);
    if (buffer == NULL) {
        free(data);
        fclose(fp);
        return -1;
    }
    
    byte_len = fread(buffer, 1, byte_len, fp);
    fclose(fp);
    
    if (byte_len <= 0) {
        free(buffer);
        free(data);
        return -1;
    }
    
    bit_len = bytes_to_bits(buffer, byte_len, data);
    free(buffer);
    
    *bits = data;
    return bit_len;
#endif
}

/**
 * @brief 打印比特序列（调试用）
 * @param bits 比特数据
 * @param n 序列长度
 * @param max_print 最大打印数量
 */
void print_bits(const BitData *bits, int n, int max_print)
{
    int i;
    int count = (n < max_print) ? n : max_print;
    
    printf("Bits (first %d of %d): ", count, n);
    for (i = 0; i < count; i++) {
        printf("%d", GET_BIT(bits, i));
    }
    if (n > max_print) {
        printf("...");
    }
    printf("\n");
}

/* ============================================================================
 * 输出格式化
 * ============================================================================ */

/**
 * @brief 将测试结果输出为JSON格式
 */
int output_json(FILE *fp, const TestSummary *summary, const char *filename)
{
    int i;
    time_t now;
    char time_str[64];
    
    if (fp == NULL || summary == NULL) {
        return -1;
    }
    
    time(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"version\": \"%s\",\n", GMRTEST_VERSION);
    fprintf(fp, "  \"timestamp\": \"%s\",\n", time_str);
    if (filename) {
        fprintf(fp, "  \"file\": \"%s\",\n", filename);
    }
    fprintf(fp, "  \"sequence_length\": %d,\n", summary->results_count > 0 ? 0 : 0);
    fprintf(fp, "  \"alpha\": 0.01,\n");
    fprintf(fp, "  \"results\": [\n");
    
    for (i = 0; i < summary->results_count; i++) {
        const TestResult *r = &summary->results[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"test\": \"%s\",\n", r->id ? r->id : "unknown");
        fprintf(fp, "      \"name\": \"%s\",\n", r->name ? r->name : "unknown");
        fprintf(fp, "      \"p_value\": %.10f,\n", r->p_value);
        fprintf(fp, "      \"q_value\": %.10f,\n", r->q_value);
        fprintf(fp, "      \"stat_value\": %.10f,\n", r->stat_value);
        fprintf(fp, "      \"passed\": %s\n", r->status == TEST_SUCCESS ? "true" : "false");
        fprintf(fp, "    }%s\n", i < summary->results_count - 1 ? "," : "");
    }
    
    fprintf(fp, "  ],\n");
    fprintf(fp, "  \"summary\": {\n");
    fprintf(fp, "    \"total_tests\": %d,\n", summary->total_tests);
    fprintf(fp, "    \"passed_tests\": %d,\n", summary->passed_tests);
    fprintf(fp, "    \"failed_tests\": %d,\n", summary->failed_tests);
    fprintf(fp, "    \"error_tests\": %d,\n", summary->error_tests);
    fprintf(fp, "    \"overall_passed\": %s\n", 
            (summary->failed_tests == 0 && summary->error_tests == 0) ? "true" : "false");
    fprintf(fp, "  }\n");
    fprintf(fp, "}\n");
    
    return 0;
}

/**
 * @brief 将测试结果输出为文本格式
 */
int output_text(FILE *fp, const TestSummary *summary)
{
    int i;
    
    if (fp == NULL || summary == NULL) {
        return -1;
    }
    
    fprintf(fp, "\n");
    fprintf(fp, "===========================================================================\n");
    fprintf(fp, "                         测试结果汇总                                     \n");
    fprintf(fp, "===========================================================================\n");
    fprintf(fp, "\n");
    fprintf(fp, "%-25s %-12s %-12s %-12s %-8s\n", "测试名称", "P值", "Q值", "统计量", "结果");
    fprintf(fp, "---------------------------------------------------------------------------\n");
    
    for (i = 0; i < summary->results_count; i++) {
        const TestResult *r = &summary->results[i];
        const char *status_str;
        
        switch (r->status) {
            case TEST_SUCCESS: status_str = "通过"; break;
            case TEST_FAILURE: status_str = "失败"; break;
            case TEST_ERROR:   status_str = "错误"; break;
            default:           status_str = "未知"; break;
        }
        
        fprintf(fp, "%-25s %-12.6f %-12.6f %-12.6f %-8s\n", 
                r->name ? r->name : "unknown",
                r->p_value, 
                r->q_value,
                r->stat_value, 
                status_str);
    }
    
    fprintf(fp, "---------------------------------------------------------------------------\n");
    fprintf(fp, "\n");
    fprintf(fp, "总计: %d 项测试\n", summary->total_tests);
    fprintf(fp, "  通过: %d\n", summary->passed_tests);
    fprintf(fp, "  失败: %d\n", summary->failed_tests);
    fprintf(fp, "  错误: %d\n", summary->error_tests);
    fprintf(fp, "\n");
    
    if (summary->failed_tests == 0 && summary->error_tests == 0) {
        fprintf(fp, "结论: 所有测试通过，序列具有随机性\n");
    } else {
        fprintf(fp, "结论: 存在失败或错误的测试，序列可能不满足随机性要求\n");
    }
    
    fprintf(fp, "===========================================================================\n");
    
    return 0;
}

/**
 * @brief 将测试结果输出为CSV格式
 */
int output_csv(FILE *fp, const TestSummary *summary)
{
    int i;
    
    if (fp == NULL || summary == NULL) {
        return -1;
    }
    
    fprintf(fp, "test_id,test_name,p_value,q_value,stat_value,status\n");
    
    for (i = 0; i < summary->results_count; i++) {
        const TestResult *r = &summary->results[i];
        const char *status_str;
        
        switch (r->status) {
            case TEST_SUCCESS: status_str = "pass"; break;
            case TEST_FAILURE: status_str = "fail"; break;
            case TEST_ERROR:   status_str = "error"; break;
            default:           status_str = "unknown"; break;
        }
        
        fprintf(fp, "%s,%s,%.10f,%.10f,%.10f,%s\n",
                r->id ? r->id : "unknown",
                r->name ? r->name : "unknown",
                r->p_value,
                r->q_value,
                r->stat_value,
                status_str);
    }
    
    return 0;
}

/* ============================================================================
 * 内存管理
 * ============================================================================ */

TestResult* results_alloc(int count)
{
    return (TestResult *)calloc(count, sizeof(TestResult));
}

void results_free(TestResult *results)
{
    if (results) {
        free(results);
    }
}

int summary_init(TestSummary *summary, int max_results)
{
    if (summary == NULL) {
        return -1;
    }
    
    if (max_results < MAX_RESULTS) {
        max_results = MAX_RESULTS;
    }
    
    summary->results = results_alloc(max_results);
    if (summary->results == NULL) {
        return -1;
    }
    
    summary->total_tests = 0;
    summary->passed_tests = 0;
    summary->failed_tests = 0;
    summary->error_tests = 0;
    summary->results_count = 0;
    
    return 0;
}

void summary_free(TestSummary *summary)
{
    if (summary) {
        if (summary->results) {
            free(summary->results);
            summary->results = NULL;
        }
    }
}

/* ============================================================================
 * 错误处理
 * ============================================================================ */

const char* get_error_message(TestStatus status)
{
    switch (status) {
        case TEST_SUCCESS: return "测试通过";
        case TEST_FAILURE: return "测试失败";
        case TEST_ERROR:   return "测试错误";
        default:           return "未知状态";
    }
}

/* ============================================================================
 * 时间测量
 * ============================================================================ */

long long get_timestamp_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/* ============================================================================
 * 字符串处理
 * ============================================================================ */

char* str_trim(char *str)
{
    char *end;
    
    if (str == NULL) {
        return NULL;
    }
    
    /* 去除前导空白 */
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    if (*str == '\0') {
        return str;
    }
    
    /* 去除尾部空白 */
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    *(end + 1) = '\0';
    
    return str;
}

int parse_test_list(const char *str, char **test_ids, int max_ids)
{
    /* TODO: 实现测试列表解析 */
    (void)str;
    (void)test_ids;
    (void)max_ids;
    return 0;
}
