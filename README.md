# gmrtest - 国密随机数检测工具

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-green.svg)]()

**gmrtest** (GuoMi Random TEST) 是一个实现国密随机数检测规范要求的随机性统计检测工具。

## 功能特性

- 完整实现 **GM/T 0005-2021** 国密随机性检测规范要求的全部 **27 项独立检测项**（15 种检测方法）
- 兼容 NIST SP 800-22 检测体系，可扩展额外项（已预留桩函数）
- 双重判定（P_value / Q_value）支持
- 纯 C 语言实现，无外部依赖
- 支持命令行界面，易于集成到自动化测试流程
- 支持多种输出格式（文本、JSON、CSV）
- 支持位压缩存储模式（BIT_PACKING=1 编译宏）
- 模块化设计，便于扩展和维护

## 支持的检测算法

GM/T 0005-2021 标准要求的全部 27 项检测项均已实现：

| 编号 | 测试ID | 测试名称 |
|------|--------|----------|
| 1 | `frequency` | 单比特频数检测 |
| 2 | `block_frequency-m=10000` | 块内频数检测 |
| 3-4 | `poker-m=4`, `poker-m=8` | 扑克检测 |
| 5-6 | `serial-m=3`, `serial-m=5` | 重叠子序列检测（每项含 P1/P2 双 P 值） |
| 7 | `runs` | 游程总数检测 |
| 8 | `runs_distribution` | 游程分布检测 |
| 9-10 | `longest_run-m=10000,ones/zeros` | 块内最大游程检测 |
| 11-12 | `binary_derivative-k=3/7` | 二元推导检测 |
| 13-16 | `autocorrelation-d=1/2/8/16` | 自相关检测 |
| 17 | `rank` | 矩阵秩检测（32×32） |
| 18-19 | `cumulative_sums-forward/backward` | 累加和检测 |
| 20-21 | `approximate_entropy-m=2/5` | 近似熵检测 |
| 22-23 | `linear_complexity-m=500/1000` | 线性复杂度检测 |
| 24 | `universal` | Maurer 通用统计检测（L=7,Q=1280） |
| 25 | `dft` | 离散傅里叶变换检测 |

> 注：序号 5/6 的 serial 检测项每项产生 2 个独立 P 值，故 25 个 ID 对应 27 项独立检测。
> 此外注册了 4 项 NIST 扩展桩（non_overlapping_template、overlapping_template、random_excursions、random_excursions_variant）。

## 快速开始

### 构建

```bash
# 进入项目目录后构建（单一版本，内置多线程支持，默认单线程运行）
make

# 调试模式构建
make DEBUG=1
```

> 多线程支持始终编译启用，无需额外构建参数。运行时通过 `-j/--threads` 指定线程数，默认单线程执行。

### 安装

```bash
# 安装到系统（需要 root 权限）
sudo make install

# 卸载
sudo make uninstall
```

### 使用

```bash
# 显示帮助
./bin/gmrtest -h

# 显示版本
./bin/gmrtest -v

# 列出所有可用测试
./bin/gmrtest -l

# 运行所有测试（默认单线程）
./bin/gmrtest -f random.bin

# 运行 GM/T 0005-2021 全部 27 项国密标准检测（默认单线程）
./bin/gmrtest -f random.bin -s gm

# 运行快速子集（排除矩阵秩/DFT/通用统计/线性复杂度等慢项，22 项）
./bin/gmrtest -f random.bin -s quick

# 8 线程并行运行国密标准检测（线性复杂度/通用统计等慢项显著提速）
./bin/gmrtest -f random.bin -s gm -j 8

# 自动线程数（读取环境变量 GMRTEST_THREADS，未设置则为单线程）
./bin/gmrtest -f random.bin -s gm -j 0

# 自定义组合请使用配置文件（逐项 0/1 开关）
./bin/gmrtest -f random.bin -c config.ini

# 指定显著性水平
./bin/gmrtest -f random.bin -a 0.05

# JSON 格式输出
./bin/gmrtest -f random.bin -F json -o result.json

# CSV 格式输出
./bin/gmrtest -f random.bin -F csv -o result.csv

# 批量检测多个文件（-f 可重复）
./bin/gmrtest -f a.bin -f b.bin -f c.bin -s gm

# 批量检测目录下所有 .bin 文件
./bin/gmrtest -d ./samples -s gm

# 批量输出合并到单个 JSON 文件（对象数组）
./bin/gmrtest -d ./samples -F json -o report.json

# 批量输出合并到单个 CSV 文件（带 filename 列）
./bin/gmrtest -d ./samples -F csv -o report.csv
```

## 命令行选项

| 选项 | 长选项 | 参数 | 说明 |
|------|--------|------|------|
| `-f` | `--file` | `<文件>` | 输入文件路径（二进制格式，可重复指定多个） |
| `-d` | `--dir` | `<目录>` | 扫描目录下所有 `.bin` 文件（可与 `-f` 混用） |
| `-o` | `--output` | `<文件>` | 输出文件路径（批量时合并写入一个文件；默认标准输出） |
| `-c` | `--config` | `<文件>` | 配置文件路径（INI 格式） |
| `-s` | `--suite` | `<名称>` | 测试套件: `all`(全部,默认), `gm`(国密27项), `quick`(快速子集) |
| `-a` | `--alpha` | `<值>` | 显著性水平（默认 0.01） |
| `-F` | `--format` | `<格式>` | 输出格式: text, json, csv |
| `-j` | `--threads` | `<值>` | 并行线程数: 0=自动(默认，读 GMRTEST_THREADS，未设置则单线程), N=并行 |
| `-l` | `--list` | - | 列出所有可用测试 |
| `-q` | `--quiet` | - | 静默模式 |
| `-h` | `--help` | - | 显示帮助信息 |
| `-v` | `--version` | - | 显示版本信息 |

## 输入格式

输入文件应为二进制格式，每个字节包含 8 个比特位。程序会自动将字节数据转换为比特序列进行检测。

### 示例：生成测试数据

```bash
# 使用 /dev/urandom 生成 1MB 随机数据
dd if=/dev/urandom of=random.bin bs=1024 count=1024

# 运行测试
./bin/gmrtest -f random.bin
```

## 输出格式

### 文本格式（默认）

```
=================================================================
                      测试结果汇总                              
=================================================================

测试名称                  P值          Q值          统计量       结果
-----------------------------------------------------------------
单比特频数检测             0.789123     0.394562     0.123456     通过
块内频数检测               0.456789     0.456789     12.345678    通过
...
-----------------------------------------------------------------

总计: 27 项测试
  通过: 26
  失败: 1
  错误: 0

结论: 存在失败或错误的测试，序列可能不满足随机性要求
=================================================================
```

### JSON 格式

```json
{
  "version": "1.0.0",
  "timestamp": "2024-01-01T12:00:00Z",
  "file": "random.bin",
  "alpha": 0.01,
  "results": [
    {
      "test": "frequency",
      "name": "单比特频数检测",
      "p_value": 0.789123,
      "q_value": 0.394562,
      "stat_value": 0.123456,
      "passed": true
    }
  ],
  "summary": {
    "total_tests": 27,
    "passed_tests": 26,
    "failed_tests": 1,
    "error_tests": 0,
    "overall_passed": false
  }
}
```

## 项目结构

```
gmrtest/
├── include/              # 头文件
│   ├── gmrtest.h        # 主 API 头文件
│   ├── types.h          # 类型定义
│   ├── config.h         # 配置常量
│   ├── tests.h          # 测试函数声明
│   ├── utils.h          # 工具函数声明
│   ├── fft.h            # FFT 接口
│   ├── matrix.h         # 矩阵（GF(2) 秩）接口
│   └── thread_pool.h    # 线程池接口
├── src/                 # 源代码
│   ├── main.c           # 命令行入口
│   ├── gmrtest.c        # API 实现
│   ├── tests/           # 测试算法实现（16 个检测）
│   │   ├── frequency.c  # 单比特频数检测
│   │   ├── block_frequency.c
│   │   ├── poker.c
│   │   ├── serial.c
│   │   ├── runs.c
│   │   ├── runs_distribution.c
│   │   ├── longest_run.c
│   │   ├── binary_derivative.c
│   │   ├── self_correlation.c
│   │   ├── rank.c
│   │   ├── cumulative_sums.c
│   │   ├── approximate_entropy.c
│   │   ├── linear_complexity.c
│   │   ├── universal.c
│   │   ├── dft.c
│   │   └── tests_stub.c # NIST 扩展桩
│   └── utils/           # 工具函数
│       ├── math.c       # 数学函数（erfc/igamc 等）
│       ├── io.c         # 输入输出
│       ├── matrix.c     # 矩阵运算
│       ├── fft.c        # 快速傅里叶变换
│       └── thread_pool.c # 线程池
├── docs/                # 文档
├── test/                # 测试数据
├── Makefile             # 构建脚本
└── README.md            # 说明文档
```

## API 使用

gmrtest 也可以作为库使用：

```c
#include "gmrtest.h"

int main(void)
{
    BitData *data;
    TestSummary summary;
    TestConfig config;
    int n;
    
    /* 读取数据 */
    n = read_bits_from_file_alloc("random.bin", &data);
    
    /* 初始化配置 */
    gmrtest_default_config(&config);
    config.alpha = 0.01;
    
    /* 初始化结果 */
    summary_init(&summary, TEST_COUNT);
    
    /* 运行所有测试 */
    gmrtest_run_all(data, n, &summary, &config);
    
    /* 输出结果 */
    output_text(stdout, &summary, NULL);
    
    /* 清理 */
    summary_free(&summary);
    free(data);
    
    return 0;
}
```

## 开发状态

- [x] 项目框架搭建
- [x] GM/T 0005-2021 全部 27 项独立检测项实现
- [x] 双重判定（P_value/Q_value）支持
- [x] 位压缩存储模式（BIT_PACKING=1）
- [x] `--suite` 测试套件（all / gm / quick）
- [x] 批量检测（`-f` 可重复、`-d/--dir` 目录扫描，text/json/csv 批量输出）
- [ ] NIST 扩展项（non_overlapping_template / overlapping_template / random_excursions / random_excursions_variant）完整算法

## 参考资料

- NIST SP 800-22: A Statistical Test Suite for Random and Pseudorandom Number Generators
- GM/T 0005-2021 随机性检测规范

## 作者

[wuliangxiaoseng](https://github.com/wuliangxiaoseng)

## 许可证

本项目基于 [MIT License](LICENSE) 开源协议发布。

Copyright (c) 2026 wuliangxiaoseng

## 贡献

欢迎提交 Issue 和 Pull Request。
