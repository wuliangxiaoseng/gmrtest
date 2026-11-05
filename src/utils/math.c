/**
 * @file math.c
 * @brief 数学工具函数实现
 * 参考: Cephes 数学库
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include <stdio.h>
#include "utils.h"

/* ============================================================================
 * 常量定义
 * ============================================================================ */

static const double MACHEP = 1.11022302462515654042E-16;  /* 2^-53 */
static const double MAXLOG = 7.09782712893383996732224E2;  /* log(MAXNUM) */
static const double MAXNUM = 1.7976931348623158E308;      /* 2^1024*(1-MACHEP) */
static const double PI = 3.14159265358979323846;

static const double big = 4.503599627370496e15;
static const double biginv = 2.22044604925031308085e-16;

/* sgngam 为线程局部变量（C11 _Thread_local）：
 * 多线程并行执行多个检测项时，各线程的 math_lgam 并发读写该标志，
 * 若为普通全局变量会构成数据竞争（ThreadSanitizer 已确认）。
 * 改为线程局部后每个线程拥有独立副本，消除竞争。 */
static _Thread_local int sgngam = 0;

/* ============================================================================
 * 辅助函数声明
 * ============================================================================ */

static double math_igam(double a, double x);
static double math_erf(double x);
static double math_polevl(double x, double *coef, int N);
static double math_p1evl(double x, double *coef, int N);

/* ============================================================================
 * 不完全伽马函数 igam(a, x)
 * ============================================================================ */

static double math_igam(double a, double x)
{
    double ans, ax, c, r;

    if ((x <= 0) || (a <= 0))
        return 0.0;

    if ((x > 1.0) && (x > a))
        return 1.0 - math_igamc(a, x);

    /* Compute  x**a * exp(-x) / gamma(a)  */
    ax = a * log(x) - x - math_lgam(a);
    if (ax < -MAXLOG) {
        return 0.0;
    }
    ax = exp(ax);

    /* power series */
    r = a;
    c = 1.0;
    ans = 1.0;

    do {
        r += 1.0;
        c *= x / r;
        ans += c;
    } while (c / ans > MACHEP);

    return ans * ax / a;
}

/* ============================================================================
 * 不完全伽马函数的补函数 igamc(a, x) = 1 - igam(a, x)
 * ============================================================================ */

double math_igamc(double a, double x)
{
    double ans, ax, c, yc, r, t, y, z;
    double pk, pkm1, pkm2, qk, qkm1, qkm2;

    if ((x <= 0) || (a <= 0))
        return 1.0;

    if ((x < 1.0) || (x < a))
        return 1.0 - math_igam(a, x);

    ax = a * log(x) - x - math_lgam(a);

    if (ax < -MAXLOG) {
        return 0.0;
    }
    ax = exp(ax);

    /* continued fraction */
    y = 1.0 - a;
    z = x + y + 1.0;
    c = 0.0;
    pkm2 = 1.0;
    qkm2 = x;
    pkm1 = x + 1.0;
    qkm1 = z * x;
    ans = pkm1 / qkm1;

    do {
        c += 1.0;
        y += 1.0;
        z += 2.0;
        yc = y * c;
        pk = pkm1 * z - pkm2 * yc;
        qk = qkm1 * z - qkm2 * yc;
        if (qk != 0) {
            r = pk / qk;
            t = fabs((ans - r) / r);
            ans = r;
        } else
            t = 1.0;
        pkm2 = pkm1;
        pkm1 = pk;
        qkm2 = qkm1;
        qkm1 = qk;
        if (fabs(pk) > big) {
            pkm2 *= biginv;
            pkm1 *= biginv;
            qkm2 *= biginv;
            qkm1 *= biginv;
        }
    } while (t > MACHEP);

    return ans * ax;
}

/* ============================================================================
 * Logarithm of gamma function
 * ============================================================================ */

/* A[]: Stirling's formula expansion of log gamma
 * B[], C[]: log gamma function between 2 and 3
 */
static unsigned short A[] = {
    0x6661,0x2733,0x9850,0x3f4a,
    0xe943,0xb580,0x7fbd,0xbf43,
    0x5ebb,0x20dc,0x019f,0x3f4a,
    0xa5a1,0x16b0,0xc16c,0xbf66,
    0x554b,0x5555,0x5555,0x3fb5
};
static unsigned short B[] = {
    0x6761,0x8ff3,0x8901,0xc095,
    0xb93e,0x355b,0xf234,0xc0e2,
    0x89e5,0xf890,0x3d73,0xc114,
    0xdb51,0xf994,0xbc82,0xc131,
    0xf20b,0x0219,0x4589,0xc13a,
    0x055e,0x5418,0x0c67,0xc12a
};
static unsigned short C[] = {
    0x12b2,0x1cf3,0xfd0d,0xc075,
    0xd757,0x7b89,0xaa0d,0xc0d0,
    0x4c9b,0xb974,0xeb84,0xc10a,
    0x0043,0x7195,0x6286,0xc131,
    0xf34c,0x892f,0x5255,0xc143,
    0xe14a,0x6a11,0xce4b,0xc13e
};

#define MAXLGM 2.556348e305

double math_lgam(double x)
{
    double p, q, u, w, z;
    int i;

    sgngam = 1;

    if (x < -34.0) {
        q = -x;
        w = math_lgam(q);
        p = floor(q);
        if (p == q) {
            return sgngam * MAXNUM;
        }
        i = (int)p;
        if ((i & 1) == 0)
            sgngam = -1;
        else
            sgngam = 1;
        z = q - p;
        if (z > 0.5) {
            p += 1.0;
            z = p - q;
        }
        z = q * sin(PI * z);
        if (z == 0.0) {
            return sgngam * MAXNUM;
        }
        z = log(PI) - log(z) - w;
        return z;
    }

    if (x < 13.0) {
        z = 1.0;
        p = 0.0;
        u = x;
        while (u >= 3.0) {
            p -= 1.0;
            u = x + p;
            z *= u;
        }
        while (u < 2.0) {
            if (u == 0.0) {
                return sgngam * MAXNUM;
            }
            z /= u;
            p += 1.0;
            u = x + p;
        }
        if (z < 0.0) {
            sgngam = -1;
            z = -z;
        } else
            sgngam = 1;
        if (u == 2.0)
            return log(z);
        p -= 2.0;
        x = x + p;
        p = x * math_polevl(x, (double *)B, 5) / math_p1evl(x, (double *)C, 6);
        return log(z) + p;
    }

    if (x > MAXLGM) {
        return sgngam * MAXNUM;
    }

    q = (x - 0.5) * log(x) - x + log(sqrt(2 * PI));
    if (x > 1.0e8)
        return q;

    p = 1.0 / (x * x);
    if (x >= 1000.0)
        q += ((7.9365079365079365079365e-4 * p
               - 2.7777777777777777777778e-3) * p
              + 0.0833333333333333333333) / x;
    else
        q += math_polevl(p, (double *)A, 4) / x;

    return q;
}

/* ============================================================================
 * Complementary error function
 * ============================================================================ */

double math_erfc(double x)
{
    static const double one_sqrtpi = 0.564189583547756287;
    double a = 1, b = x, c = x, d = x * x + 0.5;
    double q1, q2 = b / d, n = 1.0, t;

    if (fabs(x) < 2.2)
        return 1.0 - math_erf(x);
    if (x < 0)
        return 2.0 - math_erfc(-x);

    do {
        t = a * n + b * x;
        a = b;
        b = t;
        t = c * n + d * x;
        c = d;
        d = t;
        n += 0.5;
        q1 = q2;
        q2 = b / d;
    } while (fabs(q1 - q2) / q2 > MACHEP);

    return one_sqrtpi * exp(-x * x) * q2;
}

/* ============================================================================
 * Error function
 * ============================================================================ */

double math_erf(double x)
{
    static const double two_sqrtpi = 1.128379167095512574;
    double sum = x, term = x, xsqr = x * x;
    int j = 1;

    if (fabs(x) > 2.2)
        return 1.0 - math_erfc(x);

    do {
        term *= xsqr / j;
        sum -= term / (2 * j + 1);
        j++;
        term *= xsqr / j;
        sum += term / (2 * j + 1);
        j++;
    } while (fabs(term) / fabs(sum) > MACHEP);

    return two_sqrtpi * sum;
}

/* ============================================================================
 * 标准正态分布累积函数 Phi(x)
 * Phi(x) = (1 + erf(x / sqrt(2))) / 2
 * 累加和检测使用
 * ============================================================================ */

double math_normal(double x)
{
    double arg, result, sqrt2 = 1.414213562373095048801688724209698078569672;

    if (x > 0) {
        arg = x / sqrt2;
        result = 0.5 * (1 + math_erf(arg));
    } else {
        arg = -x / sqrt2;
        result = 0.5 * (1 - math_erf(arg));
    }

    return result;
}

/* ============================================================================
 * 多项式求值函数
 * ============================================================================ */

static double math_polevl(double x, double *coef, int N)
{
    double ans;
    int i;
    double *p;

    p = coef;
    ans = *p++;
    i = N;

    do
        ans = ans * x + *p++;
    while (--i);

    return ans;
}

static double math_p1evl(double x, double *coef, int N)
{
    double ans;
    double *p;
    int i;

    p = coef;
    ans = x + *p++;
    i = N - 1;

    do
        ans = ans * x + *p++;
    while (--i);

    return ans;
}