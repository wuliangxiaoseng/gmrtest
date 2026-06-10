/**
 * @file math.c
 * @brief 数学函数实现
 * @author gmrtest
 * @date 2024
 */

#include <math.h>
#include "utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

/**
 * @brief 互补误差函数 erfc
 * 使用近似公式计算
 */
double math_erfc(double x)
{
    double t, ty, result;
    
    if (x < 0.0) {
        return 2.0 - math_erfc(-x);
    }
    
    if (x > 26.0) {
        return 0.0;
    }
    
    t = 1.0 / (1.0 + 0.5 * x);
    ty = t * t;
    
    /* Horner's method for the approximation */
    result = t * exp(-x * x - 1.26551223 + 
                     t * (1.00002368 + 
                     t * (0.37409196 + 
                     t * (0.09678418 + 
                     t * (-0.18628806 + 
                     t * (0.27886807 + 
                     t * (-1.13520398 + 
                     t * (1.48851587 + 
                     t * (-0.82215223 + 
                     t * 0.17087277)))))))));
    
    return result;
}

/**
 * @brief 对数伽马函数 lgam
 * 使用Lanczos近似
 */
double math_lgam(double x)
{
    double a, b, c, d, e, f, g, h;
    
    if (x <= 0.0) {
        return 1.0; /* 错误情况 */
    }
    
    if (x < 7.0) {
        return math_lgam(x + 1.0) - log(x);
    }
    
    /* Stirling's approximation for large x */
    a = 1.0 / x;
    b = a * a;
    c = b * b;
    d = b * c;
    
    e = 0.99999999999980993 + 
        676.5203681218851 / x - 
        1259.1392167224028 / (x + 1.0) + 
        771.32342877765313 / (x + 2.0) - 
        176.61502916214059 / (x + 3.0) + 
        12.507343278686905 / (x + 4.0) - 
        0.13857109526572012 / (x + 5.0) + 
        9.9843695780195716e-6 / (x + 6.0) + 
        1.5056327351493116e-7 / (x + 7.0);
    
    f = (x - 0.5) * log(x + 6.5) - (x + 6.5) + log(M_SQRT2 * M_PI) + log(e);
    
    return f;
}

/**
 * @brief 不完全伽马函数的补函数 igamc(a, x) = 1 - igam(a, x)
 * 使用级数展开和连分数展开
 * 参考: Cephes 数学库
 */
double math_igamc(double a, double x)
{
    double ax, r, s, t, y, pk, pkm1, pkm2, qk, qkm1, qkm2;
    int k;
    
    if (x < 0.0 || a <= 0.0) {
        return 1.0;
    }
    
    if (x == 0.0) {
        return 1.0;
    }
    
    ax = a * log(x) - x - math_lgam(a);
    
    if (ax < -700.0) {
        return 0.0;
    }
    
    if (x < 1.0 + a) {
        /* 使用级数展开 */
        r = exp(ax);
        s = 1.0 / a;
        t = s;
        for (k = 1; k <= 200; k++) {
            t *= x / (a + k);
            s += t;
            if (fabs(t) < fabs(s) * 1e-15) {
                break;
            }
        }
        return 1.0 - r * s;
    }
    
    /* 使用连分数展开 (Lentz 算法) */
    y = 1.0 - a;
    pk = 1.0;
    pkm1 = 1.0;
    pkm2 = 0.0;  /* 初始化为 0 */
    qk = x;
    qkm1 = 1.0;
    qkm2 = 0.0;  /* 初始化为 0 */
    
    for (k = 1; k <= 200; k++) {
        /* 连分数系数 */
        double aa, bb;
        
        /* a_k = k * (a - k) */
        aa = k * y;
        /* b_k = (2k - 1) + x - a */
        bb = 2.0 * k - 1.0 + x - a;
        
        /* 递推 */
        pk = bb * pkm1 + aa * pkm2;
        qk = bb * qkm1 + aa * qkm2;
        
        /* 防止溢出 */
        if (fabs(pk) > 1e100 || fabs(qk) > 1e100) {
            pk /= 1e100;
            qk /= 1e100;
            pkm1 /= 1e100;
            qkm1 /= 1e100;
        }
        
        pkm2 = pkm1;
        pkm1 = pk;
        qkm2 = qkm1;
        qkm1 = qk;
        
        /* 检查收敛 */
        if (fabs(pk - pkm1) < 1e-15 * fabs(pk) && 
            fabs(qk - qkm1) < 1e-15 * fabs(qk)) {
            break;
        }
    }
    
    return exp(ax) * pk / qk;
}

/**
 * @brief 标准正态分布累积函数
 */
double math_normal(double x)
{
    return 0.5 * (1.0 + erf(x / M_SQRT2));
}

/**
 * @brief 伽马函数
 */
double math_gamma(double x)
{
    if (x <= 0.0) {
        return 1.0;
    }
    return exp(math_lgam(x));
}
