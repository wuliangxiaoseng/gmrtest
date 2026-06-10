# NIST 随机性统计测试套件 - 算法分析文档

## 目录

1. [项目概述](#项目概述)
2. [架构设计](#架构设计)
3. [检测算法详细分析](#检测算法详细分析)
4. [数据结构定义](#数据结构定义)
5. [配置选项](#配置选项)
6. [性能指标](#性能指标)
7. [新框架设计建议](#新框架设计建议)

---

## 1. 项目概述

RandomTest 是一个基于 Qt 的 GUI 应用，实现了 **NIST SP 800-22 随机性统计测试套件**，用于评估二进制序列的随机性。

### 核心特性

- **支持 18+ 项统计检测**
- 基于 Qt 5+ 的跨平台 GUI
- C/C++ 混合实现
- 显著性水平 `ALPHA = 0.01`

### 数据流

```
Binary File → BytesToBitSequence() → BitSequence* → 各项 Test → p-value/v-value → UI
```

---

## 2. 架构设计

### 分层结构

| 层 | 文件 | 语言 | 职责 |
|---|---|---|---|
| GUI | `main.cpp`, `mainwindow.*` | C++/Qt | 用户界面、结果展示 |
| 线程 | `detect_thread.*` | C++/Qt | 异步测试执行 |
| 统计检测 | 各 `*.c` 文件 | C | 核心算法实现 |
| 数学工具 | `cephes.c`, `erf.c`, `dfft.c` | C | 特殊函数、FFT等 |

### 全局状态约定

检测结果通过**全局变量**在模块间通信：

```c
// 全局变量命名格式
extern double <TestName>_p_Value;  // p值
extern double <TestName>_v_Value;  // 统计量值
```

---

## 3. 检测算法详细分析

### 3.1 算法汇总表

| 算法名称 | 测试ID | 函数签名 | 参数说明 |
|---------|--------|----------|----------|
| 单比特频数检测 | TEST_FREQUENCY(1) | `Frequency(n, epsilon)` | n: 序列长度 |
| 块内频数检测 | TEST_BLOCK_FREQUENCY(2) | `BlockFrequency(M, n, epsilon)` | M: 块长度 |
| 累加和检测 | TEST_CUSUM(3) | `CumulativeSums(n, epsilon)` | 前向+后向 |
| 游程总数检测 | TEST_RUNS(4) | `Runs(n, epsilon)` | - |
| 最长1游程检测 | TEST_LONGEST_RUN(5) | `LongestRunOfOnes(n, epsilon)` | - |
| 矩阵秩检测 | TEST_RANK(6) | `Rank(n, epsilon)` | 32x32矩阵 |
| 离散傅立叶检测 | TEST_FFT(7) | `DiscreteFourierTransform(n, epsilon)` | FFT分析 |
| 非重叠模板匹配 | TEST_NONPERIODIC(8) | `NonOverlappingTemplateMatchings(m, n, epsilon)` | m:模板长度 |
| 重叠模板匹配 | TEST_OVERLAPPING(9) | `OverlappingTemplateMatchings(m, n, epsilon)` | m:模板长度 |
| 通用统计检测 | TEST_UNIVERSAL(10) | `Universal(n, epsilon)` | - |
| 近似熵检测 | TEST_APEN(11) | `ApproximateEntropy(m, n, epsilon)` | m:块长度 |
| 随机游程检测 | TEST_RND_EXCURSION(12) | `RandomExcursions(n, epsilon)` | - |
| 随机游程变体 | TEST_RND_EXCURSION_VAR(13) | `RandomExcursionsVariant(n, epsilon)` | - |
| 重叠子序列检测 | TEST_SERIAL(14) | `Serial(m, n, epsilon)` | m:块长度 |
| 线性复杂度检测 | TEST_LINEARCOMPLEXITY(15) | `LinearComplexity(M, n, epsilon)` | M:子序列长度 |
| 二元推导检测 | - | `BinaryDerivate(k, n, epsilon)` | k:推导阶数 |
| 自相关检测 | - | `SelfCorrelation(d, n, epsilon)` | d:延迟 |
| 扑克检测 | - | `PokerDetect(M, n, epsilon)` | M:块长度 |
| 游程分布检测 | - | `RunsDistribution(n, epsilon)` | - |

---

### 3.2 算法详细说明

#### 3.2.1 单比特频数检测 (Frequency Test)

**功能**: 检测序列中 0 和 1 的比例是否接近 50%

**实现逻辑**:
```c
sum = Σ(2*epsilon[i] - 1)  // 将0→-1, 1→+1
s_obs = |sum| / sqrt(n)
p_value = erfc(s_obs / sqrt(2))
```

**输入**:
- `n`: 序列长度
- `epsilon`: BitSequence* 比特序列

**输出**:
- 返回值: 1=通过, 0=失败
- `Frequency_p_Value`: p值
- `Frequency_v_Value`: s_obs统计量

---

#### 3.2.2 块内频数检测 (Block Frequency Test)

**功能**: 将序列分成 M 位的块，检测每块中 1 的比例

**实现逻辑**:
```c
N = n / M  // 块数
for each block:
    pi = sum(block) / M
    sum += (pi - 0.5)^2
chi_squared = 4 * M * sum
p_value = igamc(N/2, chi_squared/2)
```

**参数**:
- `M`: 块长度（典型值: 128）
- `n`: 序列长度

**输出**:
- `BlockFrequency_p_Value`: p值
- `BlockFrequency_v_Value`: chi_squared

---

#### 3.2.3 累加和检测 (Cumulative Sums Test)

**功能**: 检测序列的累积和是否在随机游走的预期范围内

**实现逻辑**:
```c
// 计算累积和的最大值
S = 0, sup = 0, inf = 0
for each bit:
    epsilon[i] ? S++ : S--
    sup = max(sup, S), inf = min(inf, S)
z = max(sup, -inf)

// 前向测试
p_value_forward = 1 - sum1 + sum2

// 后向测试（反转序列）
p_value_backward = 1 - sum1' + sum2'
```

**输出**:
- `CumulativeSums_p_Value_forward`: 前向p值
- `CumulativeSums_p_Value_backward`: 后向p值
- 两个p值都 >= ALPHA 才算通过

---

#### 3.2.4 游程总数检测 (Runs Test)

**功能**: 检测游程（连续相同比特）的数量是否符合随机预期

**实现逻辑**:
```c
pi = S / n  // 1的比例
if |pi - 0.5| > 2/sqrt(n): p_value = 0

V = 1 + Σ(epsilon[k] != epsilon[k-1])  // 游程数
erfc_arg = |V - 2*n*pi*(1-pi)| / (2*pi*(1-pi)*sqrt(2n))
p_value = erfc(erfc_arg)
```

**输出**:
- `Runs_p_Value`: p值
- `Runs_v_Value`: erfc_arg

---

#### 3.2.5 最长1游程检测 (Longest Run of Ones Test)

**功能**: 检测序列中最长连续1的长度是否符合预期

**实现逻辑**: 根据序列长度自动选择参数：

| n范围 | K | M | V数组 |
|-------|---|---|-------|
| 128 <= n < 6272 | 3 | 8 | [1,2,3,4] |
| 6272 <= n < 750000 | 5 | 128 | [4,5,6,7,8,9] |
| n >= 750000 | 6 | 10000 | [10,11,12,13,14,15,16] |

```c
N = n / M
for each block:
    统计最长1游程 v_n_obs
    计数到 nu[]

chi2 = Σ((nu[i] - N*pi[i])^2 / (N*pi[i]))
p_value = igamc(K/2, chi2/2)
```

**输出**:
- `LongestRunOfOnes_p_Value`: p值
- `LongestRunOfOnes_v_Value`: chi2

---

#### 3.2.6 矩阵秩检测 (Rank Test)

**功能**: 将序列分成32x32矩阵，检测矩阵秩的分布

**实现逻辑**:
```c
N = n / (32*32)
for each 32x32矩阵:
    R = computeRank(matrix)
    F_32++, F_31++, F_30++  // 统计各秩频率

// 预计算概率
p_32 = 2^(32*(64-32)-1024) * product()
p_31 = 2^(31*(64-31)-1024) * product()
p_30 = 1 - p_32 - p_31

chi_squared = Σ((F_i - N*p_i)^2 / (N*p_i))
p_value = exp(-chi_squared/2)
```

**输出**:
- `Rank_p_Value`: p值
- `Rank_v_Value`: chi_squared

---

#### 3.2.7 离散傅立叶检测 (Discrete Fourier Transform Test)

**功能**: 通过FFT检测序列的周期性

**实现逻辑**:
```c
// 将比特转换为 ±1
X[i] = 2*epsilon[i] - 1

// FFT变换
FFT(X)

// 计算幅度
m[i] = sqrt(X[2i+1]^2 + X[2i+2]^2)

// 统计小于阈值的峰值数
upperBound = sqrt(2.995732274 * n)
count = Σ(m[i] < upperBound)

N_l = count, N_o = 0.95 * n/2
d = (N_l - N_o) / sqrt(n/4 * 0.95 * 0.05)
p_value = erfc(|d| / sqrt(2))
```

**输出**:
- `DiscreteFourierTransform_p_Value`: p值
- `DiscreteFourierTransform_v_Value`: d统计量

---

#### 3.2.8 非重叠模板匹配检测 (Non-overlapping Template Matching Test)

**功能**: 使用预定义的非周期模板检测序列

**实现逻辑**:
```c
N = 8, M = n / N
lambda = (M - m + 1) / 2^m

// 读取模板文件 templates/template{m}
for each template:
    for each block:
        W_obs = 匹配次数
    chi2 = Σ((W_j - lambda)^2 / varWj)
    p_value = igamc(N/2, chi2/2)
```

**参数**:
- `m`: 模板长度（需对应模板文件）
- 模板文件位置: `templates/template{m}`

**输出**: 所有模板都通过才算通过

---

#### 3.2.9 重叠模板匹配检测 (Overlapping Template Matching Test)

**功能**: 检测全1模板的重叠匹配次数

**实现逻辑**:
```c
M = 1032, N = n / M
lambda = (M - m + 1) / 2^m
eta = lambda / 2

// 计算概率
for i in 0..K:
    pi[i] = Pr(i, eta)

for each block:
    W_obs = 全1模板匹配次数
    nu[min(W_obs, 5)]++

chi2 = Σ((nu[i] - N*pi[i])^2 / (N*pi[i]))
p_value = igamc(K/2, chi2/2)
```

**参数**:
- `m`: 模板长度（全1模板）

**输出**:
- p值判断

---

#### 3.2.10 通用统计检测 (Universal Statistical Test)

**功能**: 检测序列的压缩性（熵估计）

**实现逻辑**:
```c
// 根据n自动选择L
L = 5..16 (随n增大)
Q = 10 * 2^L
K = floor(n/L) - Q

// 初始化表
for i=1..Q:
    decRep = 二进制转十进制
    T[decRep] = i

// 处理块
for i=Q+1..Q+K:
    decRep = 当前块的十进制表示
    sum += log2(i - T[decRep])
    T[decRep] = i

phi = sum / K
c = 0.7 - 0.8/L + (4 + 32/L) * K^(-3/L)/15
sigma = c * sqrt(variance[L]/K)
arg = |phi - expected_value[L]| / (sqrt(2)*sigma)
p_value = erfc(arg)
```

**预定义期望值**:
```c
expected_value[17] = {0,0,0,0,0,0,5.2177,6.1963,7.1837,8.1764,9.1723,
                      10.1700,11.1688,12.1681,13.1677,14.1675,15.1674}
variance[17] = {0,0,0,0,0,0,2.954,3.125,3.238,3.311,3.356,3.384,
                3.401,3.410,3.416,3.419,3.421}
```

**输出**:
- `Universal_p_Value`: p值
- `Universal_v_Value`: (phi - expected)/sigma

---

#### 3.2.11 近似熵检测 (Approximate Entropy Test)

**功能**: 检测序列的规律性/复杂性

**实现逻辑**:
```c
for blockSize = m, m+1:
    powLen = 2^(blockSize+1) - 1
    for each position:
        k = 二进制转十进制索引
        P[k-1]++
    
    sum = Σ(P[i] * log(P[i]/n))
    ApEn[r] = sum / n

apen = ApEn[0] - ApEn[1]
chi_squared = 2 * n * (log(2) - apen)
p_value = igamc(2^(m-1), chi_squared/2)
```

**参数**:
- `m`: 块长度（建议 m < log2(n) - 5）

**输出**:
- `ApproximateEntropy_p_Value`: p值
- `ApproximateEntropy_v_Value`: chi_squared

---

#### 3.2.12 重叠子序列检测 (Serial Test)

**功能**: 检测连续m位模式的频率分布

**实现逻辑**:
```c
// psi2函数计算
sum = Σ(P[i]^2)
psi = (sum * 2^m / n) - n

psim0 = psi2(m, n, epsilon)
psim1 = psi2(m-1, n, epsilon)
psim2 = psi2(m-2, n, epsilon)

del1 = psim0 - psim1
del2 = psim0 - 2*psim1 + psim2

p_value1 = igamc(2^(m-1)/2, del1/2)
p_value2 = igamc(2^(m-2)/2, del2/2)
```

**输出**:
- `Serial_p_Value_1`, `Serial_p_Value_2`: 两个p值
- 都 >= ALPHA 才算通过

---

#### 3.2.13 线性复杂度检测 (Linear Complexity Test)

**功能**: 检测序列的线性复杂度（最短线性反馈移位寄存器长度）

**实现逻辑**:
```c
M = 500 (子序列长度), N = n / M

for each substring:
    使用 Berlekamp-Massey 算法计算 L
    mean = M/2 + (9+sign)/36 - (M/3+2/9)/2^M
    T_ = sign * (L - mean) + 2/9
    
    // 统计到nu[]
    if T_ <= -2.5: nu[0]++
    elif ...
    
pi[7] = {0.010417, 0.03125, 0.125, 0.5, 0.25, 0.0625, 0.020833}
chi2 = Σ((nu[i] - N*pi[i])^2 / (N*pi[i]))
p_value = igamc(K/2, chi2/2)
```

**输出**:
- `LinearComplexity_p_Value`: p值
- `LinearComplexity_v_Value`: chi2

---

#### 3.2.14 随机游程检测 (Random Excursions Test)

**功能**: 检测随机游走返回零点的次数分布

**实现逻辑**:
```c
// 计算累积和
S_k[0] = 2*epsilon[0] - 1
S_k[i] = S_k[i-1] + 2*epsilon[i] - 1

// 确定周期（S_k=0的位置）
J = 周期数

// 约束检查
constraint = max(0.005*sqrt(n), 500)
if J < constraint: 跳过测试

// 统计各状态访问次数
stateX[8] = {-4,-3,-2,-1,1,2,3,4}
for each cycle:
    for each position:
        if S_k[i] in stateX:
            counter[state]++

// 计算p值（使用预定义概率）
pi[5][6] = {
    {0,0,0,0,0,0},
    {0.5, 0.25, 0.125, 0.0625, 0.03125, 0.03125},
    {0.75, 0.0625, 0.046875, 0.03515625, 0.0263671875, 0.0791015625},
    {0.833333, 0.0277778, 0.0231481, 0.0192901, 0.0160751, 0.0803755},
    {0.875, 0.015625, 0.0136719, 0.0119629, 0.0104675, 0.0732727}
}
```

**输出**: 8个状态的p值都需 >= ALPHA

---

#### 3.2.15 随机游程变体检测 (Random Excursions Variant Test)

**功能**: 检测扩展状态范围(-9到+9)的访问次数

**实现逻辑**:
```c
stateX[18] = {-9,-8,-7,-6,-5,-4,-3,-2,-1,1,2,3,4,5,6,7,8,9}

for each state x in stateX:
    count = Σ(S_k[i] == x)
    p_value = erfc(|count - J| / sqrt(2*J*(4*|x|-2)))
```

**输出**: 18个状态的p值都需 >= ALPHA

---

#### 3.2.16 二元推导检测 (Binary Derivate Test)

**功能**: 检测k阶推导后的序列随机性

**实现逻辑**:
```c
// k阶推导
for i = 0..k-1:
    for j = 0..n-i-2:
        epsilon[j] = epsilon[j] ^ epsilon[j+1]

// 频数检测
Sn_k = Σ(2*epsilon[i] - 1)
V = |Sn_k| / sqrt(n_k)
p_value = erfc(|V| / sqrt(2))
```

**参数**:
- `k`: 推导阶数

**输出**:
- `BinaryDerivate_p_Value`: p值
- `BinaryDerivate_v_Value`: V统计量

---

#### 3.2.17 自相关检测 (Self Correlation Test)

**功能**: 检测序列在延迟d处的自相关性

**实现逻辑**:
```c
Ad = Σ(epsilon[i] ^ epsilon[i+d])  // 异或计数
V = 2 * (Ad - n_d/2) / sqrt(n_d)
p_value = erfc(|V| / sqrt(2))
```

**参数**:
- `d`: 延迟值

**输出**:
- `SelfCorrelation_p_Value`: p值
- `SelfCorrelation_v_Value`: V统计量

---

#### 3.2.18 扑克检测 (Poker Test)

**功能**: 检测M位块的分布是否均匀

**实现逻辑**:
```c
N = n / M
maxElements = 2^M

// 统计各模式频率
for each block:
    flag = 二进制转字节
    tab[flag].count++

sum_ni = Σ(count[i]^2)
V = (maxElements/N) * sum_ni - N
p_value = igamc((maxElements-1)/2, V/2)
```

**参数**:
- `M`: 块长度（M <= 8）

**输出**:
- `PokerDetect_p_Value`: p值
- `PokerDetect_v_Value`: V统计量

---

#### 3.2.19 游程分布检测 (Runs Distribution Test)

**功能**: 检测游程长度的分布

**实现逻辑**:
```c
// 期望频率
e[i] = (n - i + 3) / 2^(i+2)
k = max{i | e[i] >= 5}

// 统计0游程和1游程
for each run:
    if run == 0: gi[length]++
    else: bi[length]++

sum_bi = Σ((bi[i] - e[i])^2 / e[i])
sum_gi = Σ((gi[i] - e[i])^2 / e[i])
V = sum_bi + sum_gi

p_value = igamc(k-1, V/2)
```

**输出**:
- `RunsDistribution_p_Value`: p值
- `RunsDistribution_v_Value`: V统计量

---

## 4. 数据结构定义

### 4.1 核心类型

```c
typedef unsigned char BitSequence;  // 单个比特

typedef struct _testParameters {
    int n;                              // 序列长度
    int blockFrequencyBlockLength;      // 块内频数的块长度
    int nonOverlappingTemplateBlockLength;  // 非重叠模板长度
    int overlappingTemplateBlockLength;     // 重叠模板长度
    int serialBlockLength;              // 子序列检测块长度
    int linearComplexitySequenceLength; // 线性复杂度子序列长度
    int approximateEntropyBlockLength;  // 近似熵块长度
    int numOfBitStreams;                // 比特流数量
} TP;
```

### 4.2 全局结果变量

| 全局变量 | 对应测试 |
|---------|---------|
| `Frequency_p_Value/v_Value` | 单比特频数 |
| `BlockFrequency_p_Value/v_Value` | 块内频数 |
| `PokerDetect_p_Value/v_Value` | 扑克检测 |
| `Serial_p_Value_1/2, v_Value_1/2` | 重叠子序列 |
| `Runs_p_Value/v_Value` | 游程总数 |
| `RunsDistribution_p_Value/v_Value` | 游程分布 |
| `LongestRunOfOnes_p_Value/v_Value` | 最长1游程 |
| `BinaryDerivate_p_Value/v_Value` | 二元推导 |
| `SelfCorrelation_p_Value/v_Value` | 自相关 |
| `Rank_p_Value/v_Value` | 矩阵秩 |
| `CumulativeSums_p_Value_forward/backward, v_Value` | 累加和 |
| `ApproximateEntropy_p_Value/v_Value` | 近似熵 |
| `LinearComplexity_p_Value/v_Value` | 线性复杂度 |
| `Universal_p_Value/v_Value` | 通用统计 |
| `DiscreteFourierTransform_p_Value/v_Value` | 离散傅立叶 |

---

## 5. 配置选项

### 5.1 常量定义 (`include/defs.h`)

```c
#define ALPHA               0.01        // 显著性水平
#define MAXNUMOFTEMPLATES   148         // 最大模板数
#define NUMOFTESTS          15          // 主测试数量
#define NUMOFGENERATORS     10          // 最大PRNG数
```

### 5.2 测试ID常量

```c
#define TEST_FREQUENCY          1
#define TEST_BLOCK_FREQUENCY    2
#define TEST_CUSUM              3
#define TEST_RUNS               4
#define TEST_LONGEST_RUN        5
#define TEST_RANK               6
#define TEST_FFT                7
#define TEST_NONPERIODIC        8
#define TEST_OVERLAPPING        9
#define TEST_UNIVERSAL          10
#define TEST_APEN               11
#define TEST_RND_EXCURSION      12
#define TEST_RND_EXCURSION_VAR  13
#define TEST_SERIAL             14
#define TEST_LINEARCOMPLEXITY   15
```

---

## 6. 性能指标

### 6.1 时间复杂度

| 算法 | 时间复杂度 | 空间复杂度 |
|-----|-----------|-----------|
| Frequency | O(n) | O(1) |
| BlockFrequency | O(n) | O(1) |
| CumulativeSums | O(n) | O(1) |
| Runs | O(n) | O(1) |
| LongestRunOfOnes | O(n) | O(1) |
| Rank | O(n/1024 * 32^3) = O(n) | O(32^2) |
| DiscreteFourierTransform | O(n log n) | O(n) |
| NonOverlappingTemplateMatchings | O(n * m * num_templates) | O(m) |
| OverlappingTemplateMatchings | O(n * m) | O(m) |
| Universal | O(n) | O(2^L) |
| ApproximateEntropy | O(n * 2^m) | O(2^m) |
| Serial | O(n * 2^m) | O(2^m) |
| LinearComplexity | O(n * M) | O(M) |
| RandomExcursions | O(n) | O(n) |
| RandomExcursionsVariant | O(n) | O(n) |
| BinaryDerivate | O(k * n) | O(1) |
| SelfCorrelation | O(n) | O(1) |
| PokerDetect | O(n) | O(2^M) |
| RunsDistribution | O(n) | O(n) |

### 6.2 内存使用

- **峰值内存**: 主要由 FFT 和 Universal 测试决定
- **FFT**: 需要 O(n) 临时数组
- **Universal**: 需要 O(2^L) 查找表，L最大为16，即 65536 个 long

---

## 7. 新框架设计建议

### 7.1 设计原则

1. **纯C实现**: 移除Qt依赖，专注命令行环境
2. **模块化设计**: 每个测试独立封装
3. **标准输入输出**: 支持管道和文件重定向
4. **JSON输出**: 便于机器解析
5. **并发支持**: 支持多线程并行测试

### 7.2 目录结构

```
random1/
├── include/           # 头文件
│   ├── nist.h        # 主API
│   ├── tests.h       # 测试声明
│   ├── utils.h       # 工具函数
│   └── types.h       # 类型定义
├── src/              # 源代码
│   ├── tests/        # 各测试实现
│   │   ├── frequency.c
│   │   ├── block_frequency.c
│   │   └── ...
│   ├── utils/        # 工具函数
│   │   ├── math.c    # 数学函数
│   │   ├── fft.c     # FFT实现
│   │   └── io.c      # 输入输出
│   └── main.c        # 命令行入口
├── templates/        # 模板文件（可选）
├── Makefile
└── README.md
```

### 7.3 API设计

```c
// 测试结果结构
typedef struct {
    const char *name;
    double p_value;
    double stat_value;
    int passed;
} NIST_Result;

// 测试配置
typedef struct {
    int n;                              // 序列长度
    int block_len;                      // 通用块长度
    int template_len;                   // 模板长度
    int derivate_order;                 // 推导阶数
    int correlation_delay;              // 自相关延迟
    double alpha;                       // 显著性水平
} NIST_Config;

// 测试函数指针
typedef int (*NIST_Test)(const BitSequence *data, int n, NIST_Result *result, void *params);

// 运行单个测试
int nist_run_test(NIST_Test test, const BitSequence *data, int n, 
                  NIST_Result *result, void *params);

// 运行所有测试
int nist_run_all_tests(const BitSequence *data, int n, 
                       NIST_Result *results, int *count);
```

### 7.4 命令行接口

```bash
# 基本用法
./nist-test -f input.bin
./nist-test -f input.bin -o results.json
./nist-test -f input.bin --tests frequency,blockfrequency,cusum

# 选项
-f, --file        输入文件路径
-o, --output      输出文件路径（JSON格式）
--tests           指定要运行的测试（逗号分隔）
--alpha           显著性水平（默认0.01）
--help            显示帮助信息
```

### 7.5 输出格式

```json
{
    "version": "2.0.8",
    "timestamp": "2024-01-01T12:00:00Z",
    "file": "input.bin",
    "sequence_length": 1000000,
    "alpha": 0.01,
    "results": [
        {
            "test": "frequency",
            "name": "单比特频数检测",
            "p_value": 0.789123,
            "stat_value": 0.123456,
            "passed": true
        },
        {
            "test": "block_frequency",
            "name": "块内频数检测",
            "p_value": 0.456789,
            "stat_value": 12.345678,
            "passed": true,
            "params": { "block_length": 128 }
        }
    ],
    "summary": {
        "total_tests": 15,
        "passed_tests": 14,
        "failed_tests": 1,
        "overall_passed": false
    }
}
```

### 7.6 性能优化建议

1. **内存映射文件**: 对于大文件使用 `mmap`
2. **线程池**: 并行运行独立测试
3. **SIMD优化**: 使用SSE/NEON加速位操作
4. **缓存友好**: 优化数据访问模式

---

## 附录：测试要求的最小序列长度

| 测试 | 最小长度 | 推荐长度 |
|-----|---------|---------|
| Frequency | 100 | 100000 |
| BlockFrequency | 100 | 100000 |
| CumulativeSums | 100 | 100000 |
| Runs | 100 | 100000 |
| LongestRunOfOnes | 128 | 100000 |
| Rank | 1024 | 1000000 |
| DiscreteFourierTransform | 1000 | 1000000 |
| NonOverlappingTemplateMatchings | 8 * 2^m | 取决于模板长度 |
| OverlappingTemplateMatchings | 8256 | 1000000 |
| Universal | 387840 (L=6) | 1000000 |
| ApproximateEntropy | 100 | 100000 |
| Serial | 2^(m+2) | 100000 |
| LinearComplexity | 500 | 100000 |
| RandomExcursions | 10000 | 1000000 |
| RandomExcursionsVariant | 10000 | 1000000 |
| BinaryDerivate | 100 + k | 100000 |
| SelfCorrelation | 100 + d | 100000 |
| PokerDetect | 4 * 2^M | 100000 |
| RunsDistribution | 100 | 100000 |
