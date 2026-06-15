/**
 * @file dft.c
 * @brief 离散傅里叶变换检测实现
 * 国密规范 GM/T 0005-2021
 * 参考: NIST SP 800-22 discreteFourierTransform.c
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdlib.h>
#include "tests.h"
#include "utils.h"
#include "fft.h"

/* 取不大于 n 的最大 2 的幂 */
static int floor_pow2(int n)
{
    int p = 1;
    while ((p << 1) <= n) p <<= 1;
    return p;
}

TestStatus test_dft(const BitData *data, int n,
                    TestResult *result, const TestConfig *config)
{
    int i, count, eff_n;
    double *re = NULL, *im = NULL;
    double upperBound, N_l, N_o, d, p_value, q_value;
    double sqrt2 = 1.41421356237309504880;

    if (data == NULL || result == NULL || n <= 0 || config == NULL) {
        return TEST_ERROR;
    }

    /* 截取到不大于 n 的最大 2 的幂，保证 FFT 可用 */
    eff_n = floor_pow2(n);
    if (eff_n < 2) {
        result->id = TEST_ID_DFT_STR;
        result->name = "离散傅立叶检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "数据长度不足";
        return TEST_ERROR;
    }

    re = (double *)calloc(eff_n, sizeof(double));
    im = (double *)calloc(eff_n, sizeof(double));
    if (re == NULL || im == NULL) {
        if (re) free(re);
        if (im) free(im);
        result->id = TEST_ID_DFT_STR;
        result->name = "离散傅立叶检测";
        result->p_value = 0.0;
        result->q_value = 0.0;
        result->stat_value = 0.0;
        result->status = TEST_ERROR;
        result->message = "内存分配失败";
        return TEST_ERROR;
    }

    for (i = 0; i < eff_n; i++) {
        re[i] = 2.0 * (double)GET_BIT(data, i) - 1.0;
        im[i] = 0.0;
    }

    fft_forward(re, im, eff_n);

    /* 统计 |X[k]| < upperBound 的频点数 */
    upperBound = sqrt(2.995732274 * (double)eff_n);
    count = 0;
    for (i = 0; i < eff_n / 2; i++) {
        double mag = sqrt(re[i] * re[i] + im[i] * im[i]);
        if (mag < upperBound) count++;
    }
    N_l = (double)count;
    N_o = 0.95 * (double)eff_n / 2.0;
    d = (N_l - N_o) / sqrt((double)eff_n / 4.0 * 0.95 * 0.05);

    p_value = math_erfc(fabs(d) / sqrt2);
    q_value = math_erfc(d / sqrt2);

    free(re);
    free(im);

    result->id = TEST_ID_DFT_STR;
    result->name = "离散傅立叶检测";
    result->p_value = p_value;
    result->q_value = q_value;
    result->stat_value = d;

    if (p_value < config->alpha) {
        result->status = TEST_FAILURE;
        result->message = "p值小于显著性水平";
        return TEST_FAILURE;
    }
    result->status = TEST_SUCCESS;
    result->message = "通过";
    return TEST_SUCCESS;
}
