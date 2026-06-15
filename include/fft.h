/**
 * @file fft.h
 * @brief 复数快速傅里叶变换
 * @author gmrtest
 * @date 2024
 */

#ifndef GMRTEST_FFT_H
#define GMRTEST_FFT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 原地复数 FFT (Cooley-Tukey, n 必须为 2 的幂)
 * @param re 实部数组
 * @param im 虚部数组
 * @param n  长度（必须是 2 的幂）
 */
void fft_forward(double *re, double *im, int n);

#ifdef __cplusplus
}
#endif

#endif
