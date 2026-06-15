/**
 * @file fft.c
 * @brief Cooley-Tukey 复数 FFT 实现
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include "fft.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void fft_forward(double *re, double *im, int n)
{
    int i, j, k, m;
    double tre, tim, cosv, sinv, theta;

    /* bit-reversal */
    j = 0;
    for (i = 1; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            tre = re[i]; re[i] = re[j]; re[j] = tre;
            tim = im[i]; im[i] = im[j]; im[j] = tim;
        }
    }

    for (m = 2; m <= n; m <<= 1) {
        int half = m >> 1;
        theta = -2.0 * M_PI / (double)m;
        for (k = 0; k < n; k += m) {
            for (i = 0; i < half; i++) {
                double angle = theta * (double)i;
                cosv = cos(angle);
                sinv = sin(angle);
                tre = cosv * re[k + i + half] - sinv * im[k + i + half];
                tim = sinv * re[k + i + half] + cosv * im[k + i + half];
                re[k + i + half] = re[k + i] - tre;
                im[k + i + half] = im[k + i] - tim;
                re[k + i] += tre;
                im[k + i] += tim;
            }
        }
    }
}
