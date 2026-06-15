# GMT/0005 国密随机性检测项完整实现计划

> 项目路径：`/root/dp_workspace/random/random1`  
> 参考资料：`docs/GMT0005随机性检测规范详解.md`、`docs/add_test_guide.md`、`/root/dp_workspace/random/RandomTest/`  
> 计划日期：2026-06-14

---

## 一、Summary（计划总结）

按照 [add_test_guide.md](file:///root/dp_workspace/random/random1/docs/add_test_guide.md) 提供的标准化操作指南，补全 GM/T 0005-2021 规范要求但当前未实现的全部检测算法，并同步更新 `todo.md`、`README.md`、`include/config.h`、`include/tests.h`、`src/gmrtest.c`、`src/main.c`、`Makefile`、`test/gm_config.ini`。

国密规范共 **15 种检测方法、27 项独立检测项**。当前 [todo.md](file:///root/dp_workspace/random/random1/todo.md) 记录已完成 12 项（实际仓库 22 项注册中含 stub），尚需补全 **15 项独立检测项**（涉及 9 种检测方法）。

---

## 二、Current State Analysis（现状分析）

### 2.1 已完整实现（位于 [src/tests](file:///root/dp_workspace/random/random1/src/tests)）

| # | 测试ID | 文件 | 状态 |
|---|--------|------|------|
| 1 | `frequency` | `frequency.c` | ✅ 完整 |
| 2 | `block_frequency-m=10000` | `block_frequency.c` | ✅ 完整 |
| 3-4 | `poker-m=4`, `poker-m=8` | `poker.c` (multi_func) | ✅ 完整 |
| 5-8 | `serial-m=3`, `serial-m=5` (P1/P2) | `serial.c` (multi_func) | ✅ 完整 |
| 9 | `runs` | `runs.c` | ✅ 完整 |
| 10 | `runs_distribution` | `runs_distribution.c` | ✅ 完整 |
| 11-12 | `longest_run-m=10000,ones/zeros` | `longest_run.c` | ✅ 完整 |
| 13-14 | `binary_derivative-k=3/k=7` | `binary_derivative.c` | ✅ 完整 |

### 2.2 当前为桩实现（位于 [tests_stub.c](file:///root/dp_workspace/random/random1/src/tests/tests_stub.c)）

| 函数 | 国密项数 | 待补全 |
|------|----------|--------|
| `test_cumulative_sums` | 2（forward/backward） | ⏳ |
| `test_rank` | 1 | ⏳ |
| `test_dft` | 1 | ⏳ |
| `test_universal` | 1（L=7,Q=1280） | ⏳ |
| `test_approximate_entropy` | 2（m=2/m=5） | ⏳ |
| `test_linear_complexity` | 2（m=500/m=1000） | ⏳ |
| `test_self_correlation` | 4（d=1/2/8/16） | ⏳ |
| `test_binary_derivate` | 旧函数（保留以兼容） | — |

合计待补全国密独立检测项：**13 项**（2+1+1+1+2+2+4）。  
**已注册但仍为桩**：自相关 stub 当前只有 1 个 `test_self_correlation`，未拆分 d=1/2/8/16；累加和 stub 也未拆分。

### 2.3 待新增 / 修复的辅助内容

- [include/config.h](file:///root/dp_workspace/random/random1/include/config.h)：
  - `TEST_COUNT` 当前为 22，按计划需扩展至覆盖所有 4 个自相关 + 2 个累加和 + 2 个近似熵 + 2 个线性复杂度独立项。
  - 缺少新增检测项的字符串 ID 宏（用户已明确要求 ID 用宏定义，统一风格）。
  - 缺少最小长度宏（如 `MIN_LENGTH_DFT`、`MIN_LENGTH_UNIVERSAL` 已有，但 `linear_complexity-m=1000` 等参数化项未单独定义）。
- [include/tests.h](file:///root/dp_workspace/random/random1/include/tests.h)：缺少各参数变体函数声明（如 `test_cumulative_sums_forward/backward`、`test_self_correlation_d1/d2/d8/d16`、`test_approximate_entropy_m2/m5`、`test_linear_complexity_m500/m1000`）。
- [src/gmrtest.c](file:///root/dp_workspace/random/random1/src/gmrtest.c)：
  - 测试项注册仍使用字符串字面量（如 `"binary_derivative-k=3"`），需统一替换为宏。
  - `gmrtest_run_multiple` 中的 ID 派发逻辑需要支持新增参数变体（cumulative_sums-forward/backward、self_correlation-d=N、approximate_entropy-m=N、linear_complexity-m=N）。
- [Makefile](file:///root/dp_workspace/random/random1/Makefile)：需添加新增源文件。
- [test/gm_config.ini](file:///root/dp_workspace/random/random1/test/gm_config.ini)：默认启用项需对齐国密 27 项。
- [todo.md](file:///root/dp_workspace/random/random1/todo.md) 与 README：进度同步。

---

## 三、Proposed Changes（详细变更）

### 3.1 头文件改造

#### 文件：[include/config.h](file:///root/dp_workspace/random/random1/include/config.h)

**新增字符串 ID 宏（用户强制要求 ID 必须使用宏）**：

```c
/* 现已存在但需补齐 */
#define TEST_ID_BINARY_DERIVATIVE_K3_STR        "binary_derivative-k=3"
#define TEST_ID_BINARY_DERIVATIVE_K7_STR        "binary_derivative-k=7"
#define TEST_ID_LONGEST_RUN_M10000_ONES_STR     "longest_run-m=10000,ones"
#define TEST_ID_LONGEST_RUN_M10000_ZEROS_STR    "longest_run-m=10000,zeros"
#define TEST_ID_BLOCK_FREQUENCY_M10000_STR      "block_frequency-m=10000"
#define TEST_ID_POKER_M4_STR                    "poker-m=4"
#define TEST_ID_POKER_M8_STR                    "poker-m=8"
#define TEST_ID_SERIAL_M3_STR                   "serial-m=3"
#define TEST_ID_SERIAL_M5_STR                   "serial-m=5"

/* 新增（本次计划） */
#define TEST_ID_CUMULATIVE_SUMS_FORWARD_STR     "cumulative_sums-forward"
#define TEST_ID_CUMULATIVE_SUMS_BACKWARD_STR    "cumulative_sums-backward"
#define TEST_ID_AUTOCORRELATION_D1_STR          "autocorrelation-d=1"
#define TEST_ID_AUTOCORRELATION_D2_STR          "autocorrelation-d=2"
#define TEST_ID_AUTOCORRELATION_D8_STR          "autocorrelation-d=8"
#define TEST_ID_AUTOCORRELATION_D16_STR         "autocorrelation-d=16"
#define TEST_ID_APPROXIMATE_ENTROPY_M2_STR      "approximate_entropy-m=2"
#define TEST_ID_APPROXIMATE_ENTROPY_M5_STR      "approximate_entropy-m=5"
#define TEST_ID_LINEAR_COMPLEXITY_M500_STR      "linear_complexity-m=500"
#define TEST_ID_LINEAR_COMPLEXITY_M1000_STR     "linear_complexity-m=1000"

/* 默认参数补充 */
#define UNIVERSAL_GM_L                          7
#define UNIVERSAL_GM_Q                          1280
#define APPROX_ENTROPY_M_2                      2
#define APPROX_ENTROPY_M_5                      5
#define LINEAR_COMPLEXITY_M_500                 500
#define LINEAR_COMPLEXITY_M_1000                1000
#define AUTOCORRELATION_D_1                     1
#define AUTOCORRELATION_D_2                     2
#define AUTOCORRELATION_D_8                     8
#define AUTOCORRELATION_D_16                    16
```

`TEST_COUNT` 更新为新的注册总数（27 国密项 + DFT + ……，预计 28 ~ 30）。最终值在实现完成后回填。

#### 文件：[include/tests.h](file:///root/dp_workspace/random/random1/include/tests.h)

为每个参数变体新增包装函数声明（保持单结果接口，方便注册）：

```c
TestStatus test_cumulative_sums_forward(...);
TestStatus test_cumulative_sums_backward(...);
TestStatus test_self_correlation_d1(...);
TestStatus test_self_correlation_d2(...);
TestStatus test_self_correlation_d8(...);
TestStatus test_self_correlation_d16(...);
TestStatus test_approximate_entropy_m2(...);
TestStatus test_approximate_entropy_m5(...);
TestStatus test_linear_complexity_m500(...);
TestStatus test_linear_complexity_m1000(...);
```

保留通用函数 `test_cumulative_sums`、`test_self_correlation`、`test_approximate_entropy`、`test_linear_complexity`、`test_universal`、`test_dft`、`test_rank` 仍存在于头文件，由各 wrapper 复用。

### 3.2 算法源码（每项一份新文件，参考 RandomTest 对应实现）

按 [add_test_guide.md](file:///root/dp_workspace/random/random1/docs/add_test_guide.md) 中"§2.3 代码实现"模板编写。

| 新文件 | 内容 | 参考文件 |
|--------|------|----------|
| `src/tests/cumulative_sums.c` | 累加和检测（前向/后向）<br/>实现 `test_cumulative_sums(forward/backward)` 通用核心，通过 `config->derivate_order` 借位（或新增 `direction` 字段）区分；提供 `test_cumulative_sums_forward/backward` 包装 | `cusum.c` |
| `src/tests/rank.c` + `src/utils/matrix.c` | 32×32 二元矩阵秩检测（GF(2) 高斯消元） | `rank.c`、`matrix.c` |
| `src/tests/dft.c` + `src/utils/fft.c`（可选） | 离散傅里叶检测；FFT 用 [Cooley-Tukey 简化版] 或借鉴 `dfft.c` | `discreteFourierTransform.c`、`dfft.c` |
| `src/tests/universal.c` | Maurer 通用统计：国密 L=7,Q=1280 | `universal.c` |
| `src/tests/approximate_entropy.c` | 近似熵 m=2/m=5；提供两个 wrapper | `approximateEntropy.c` |
| `src/tests/linear_complexity.c` + `src/utils/berlekamp_massey.c` | 线性复杂度 m=500/m=1000；提供两个 wrapper | `linearComplexity.c` |
| `src/tests/self_correlation.c` | 自相关 d=1/2/8/16；提供 4 个 wrapper | `selfCorrelation.c` |

实现要点：

- **比特访问**：所有循环必须使用 `GET_BIT(data, i)`，与位压缩/字节模式兼容。
- **数学函数**：调用 `math_erfc`、`math_igamc`、`math_lgam`、`math_normal`（如 cumsum 需要 `math_normal`，需要确认其是否已经实现；若 [math.c](file:///root/dp_workspace/random/random1/src/utils/math.c) 缺失则参考 `cephes.c::cephes_normal` 移植，已有则直接调用）。
- **结果填写规范**：
  - `result->id` 使用对应的 `TEST_ID_*_STR` 宏；
  - `result->name` 写中文名；
  - `result->stat_value`、`result->p_value`、`result->q_value`；
  - 卡方型检测 `q_value = p_value`；正态型检测 `q_value = erfc(V_raw/√2)`（不取绝对值）。
- **参数封装包装函数**采用与 `test_binary_derivative_k3/k7` 同款写法：拷贝 `config`，覆写参数后调用通用核心。

#### 关键实现细节

1. **累加和方向**：`TestConfig` 已有字段不足以表达方向。计划在 [types.h](file:///root/dp_workspace/random/random1/include/types.h) 的 `TestConfig` 中**新增** `int cusum_mode;` (0=forward, 1=backward)，并在 wrapper 中设置；保持 `test_cumulative_sums` 多参数版本逻辑一致。

2. **自相关延迟**：`TestConfig.correlation_delay` 已存在；wrapper 设置 d=1/2/8/16 后调用 `test_self_correlation`。

3. **近似熵 m**：复用 `TestConfig.block_length`（与块内频数的 m=10000 不冲突，因为不同检测项执行时各自的 wrapper 会覆盖）。

4. **线性复杂度 m**：同样复用 `TestConfig.block_length`，wrapper 内覆写。

5. **DFT**：如不便引入外部 FFT，使用 `n log n` 的递归 FFT（在 `src/utils/fft.c` 中实现）；注意 `n` 必须为偶数（NIST 实现要求），不足时按规范要求截断或返回 ERROR。

### 3.3 注册表与派发逻辑

#### 文件：[src/gmrtest.c](file:///root/dp_workspace/random/random1/src/gmrtest.c)

`all_tests[TEST_COUNT]` 数组中：
- 删除/替换原 stub `cumulative_sums`、`autocorrelation` 等"通用未参数化"条目；
- 改为按 27 项独立检测项分别注册，每项使用对应的 `TEST_ID_*_STR` 宏作为 `id`，并指向对应 wrapper 函数。
- `poker-m=4/m=8` 和 `serial-m=3/m=5` 仍由 `multi_func` 提供（保留现状）。

`gmrtest_run_multiple` 中 ID 派发逻辑：
- 由于 wrapper 已经按完整 ID 注册，所有 `算法名-参数` ID 可直接通过 `get_test_by_id(test_id)` 命中，无需再通过前缀匹配。
- 仅 `poker-m=*`、`serial-m=*` 由于使用 multi_func + sub_index，保留现有处理；其他分支可清理简化。

#### 文件：[src/main.c](file:///root/dp_workspace/random/random1/src/main.c)

- 更新 `gm_tests[]` 列表为完整的国密 27 项（当前仅 10 项）。
- `nist_tests[]` 不变（已包含 27+）。
- `config_entries[]` 已有所有 ID，无需大改。
- `print_usage` 中的 INI 示例同步。

### 3.4 配置 / 构建 / 文档

#### 文件：[Makefile](file:///root/dp_workspace/random/random1/Makefile)

`SRCS_TESTS` 增加：
```makefile
$(SRC_DIR)/tests/cumulative_sums.c \
$(SRC_DIR)/tests/rank.c \
$(SRC_DIR)/tests/dft.c \
$(SRC_DIR)/tests/universal.c \
$(SRC_DIR)/tests/approximate_entropy.c \
$(SRC_DIR)/tests/linear_complexity.c \
$(SRC_DIR)/tests/self_correlation.c \
```

`SRCS_UTILS` 增加（如启用）：
```makefile
$(SRC_DIR)/utils/matrix.c \
$(SRC_DIR)/utils/fft.c \
$(SRC_DIR)/utils/berlekamp_massey.c \
```

由于新增源文件后 `tests_stub.c` 中对应的桩函数会与新实现冲突，需 **从 `tests_stub.c` 中删除被替换的桩**（保留 `test_overlapping_template`、`test_non_overlapping_template`、`test_random_excursions`、`test_random_excursions_variant`、`test_binary_derivate` 这些非国密必需项的桩）。

#### 文件：[test/gm_config.ini](file:///root/dp_workspace/random/random1/test/gm_config.ini)

将所有国密 27 项的开关默认设置为 `1`：
- `cumulative_sums-forward = 1`
- `cumulative_sums-backward = 1`
- `rank = 1`
- `dft = 1`
- `universal = 1`
- `approximate_entropy-m=2 = 1`
- `approximate_entropy-m=5 = 1`
- `linear_complexity-m=500 = 1`
- `linear_complexity-m=1000 = 1`
- `autocorrelation-d=1 = 1`
- `autocorrelation-d=2 = 1`
- `autocorrelation-d=8 = 1`
- `autocorrelation-d=16 = 1`

#### 文件：[todo.md](file:///root/dp_workspace/random/random1/todo.md)

- 27 项状态全部标记为 🆗。
- 进度条更新到 100%。
- 增加"已完成功能"清单。
- 在"参考资源"中保留对 [add_test_guide.md](file:///root/dp_workspace/random/random1/docs/add_test_guide.md) 的引用。

#### 文件：[README.md](file:///root/dp_workspace/random/random1/README.md)

- 更新支持的测试项列表（27 项国密 + 3 项 NIST 桩）。
- 更新使用示例中的 `-g` 描述（10 项 → 27 项）。

#### 文件：[docs/add_test_guide.md](file:///root/dp_workspace/random/random1/docs/add_test_guide.md)

- 在"附录：现有测试项清单"补全 13 项新增；
- 修订"§2.4.1 注册测试项"示例使用宏；
- 更新版本历史增加 1.1 行：完成 GMT/0005 全部 27 项。

---

## 四、Assumptions & Decisions（关键假设与决策）

| 决策 | 选择 | 理由 |
|------|------|------|
| 参数变体的实现方式 | 每个变体提供 wrapper 函数 + 通用核心，wrapper 在 `gmrtest.c` 的 `all_tests[]` 单独注册 | 与已实现的 `test_binary_derivative_k3/k7` 一致；可让 `get_test_by_id` 直接命中完整 ID，避免在 `gmrtest_run_multiple` 中维护大量 `strncmp` 分支 |
| 测试 ID 命名 | 统一 `算法名-参数` 字符串，并为每个 ID 定义 `TEST_ID_*_STR` 宏 | 用户明确要求 ID 使用宏定义保持代码风格一致 |
| 数学函数依赖 | 复用 `math_erfc`、`math_igamc`、`math_lgam`、`math_normal`；如缺失则从 `cephes.c` 移植 | 数值稳定，避免重写 |
| FFT 实现 | 在 `src/utils/fft.c` 内置 Cooley-Tukey FFT（处理 n 为 2 的幂；非 2 的幂时取最大 2^k） | 不引入外部依赖；与 `RandomTest/dfft.c` 思路一致 |
| 矩阵秩 | 在 `src/utils/matrix.c` 实现 GF(2) 行简化求秩 | 复刻 `rank.c::computeRank` |
| Berlekamp-Massey | 在 `src/utils/berlekamp_massey.c` 提供独立 `bm_complexity(seq, m)` | 易于单元测试，复用于 m=500/m=1000 |
| `cumsum` 方向区分 | `TestConfig` 新增 `int cusum_mode;`（保持向后兼容，默认 0） | 避免重新定义 ID 解析 |
| `tests_stub.c` 中保留哪些桩 | 保留 `test_overlapping_template`、`test_non_overlapping_template`、`test_random_excursions`、`test_random_excursions_variant`、`test_binary_derivate`（旧名） | 这些为 NIST/兼容项，国密不要求；后续低优先级再实现 |
| 输出 q_value | 卡方型 = p_value；正态型 = `erfc(V_raw/√2)`（不取绝对值） | 与 [todo.md](file:///root/dp_workspace/random/random1/todo.md) 第六章双重判定一致 |

---

## 五、Implementation Steps（执行清单）

> 执行阶段按顺序进行；每完成 1 - 2 项算法即编译验证。

1. **Phase 0 准备**
   - [ ] 在 `include/config.h` 增补字符串 ID 宏 + 默认参数宏。
   - [ ] 在 `include/types.h` 的 `TestConfig` 增加 `int cusum_mode;` 字段；更新 `gmrtest_default_config`。
   - [ ] 在 `include/tests.h` 增补 wrapper 函数声明。

2. **Phase 1 累加和（cumulative_sums）**
   - [ ] 新增 `src/tests/cumulative_sums.c`，实现核心 + 2 个 wrapper。
   - [ ] 删除 `tests_stub.c::test_cumulative_sums`。
   - [ ] 在 `gmrtest.c::all_tests[]` 注册 forward/backward 两项。
   - [ ] `make clean && make` 通过。

3. **Phase 2 自相关（self_correlation）**
   - [ ] 新增 `src/tests/self_correlation.c`，4 个 wrapper。
   - [ ] 删除 `tests_stub.c::test_self_correlation`。
   - [ ] 注册 d=1/2/8/16 四项。

4. **Phase 3 近似熵（approximate_entropy）**
   - [ ] 新增 `src/tests/approximate_entropy.c`，2 个 wrapper（m=2/m=5）。
   - [ ] 删除 stub。
   - [ ] 注册两项。

5. **Phase 4 矩阵秩（rank）**
   - [ ] 新增 `src/utils/matrix.c`：`compute_rank_gf2(uint8_t *m, int rows, int cols)`。
   - [ ] 新增 `src/tests/rank.c`。
   - [ ] 删除 stub。

6. **Phase 5 离散傅里叶（dft）**
   - [ ] 新增 `src/utils/fft.c`：`fft_forward(double *re, double *im, int n)`。
   - [ ] 新增 `src/tests/dft.c`。
   - [ ] 删除 stub。

7. **Phase 6 通用统计（universal）**
   - [ ] 新增 `src/tests/universal.c`（L=7,Q=1280；移植 NIST 表常数）。
   - [ ] 删除 stub。

8. **Phase 7 线性复杂度（linear_complexity）**
   - [ ] 新增 `src/utils/berlekamp_massey.c`。
   - [ ] 新增 `src/tests/linear_complexity.c`，2 个 wrapper。
   - [ ] 删除 stub。

9. **Phase 8 注册整理**
   - [ ] `src/gmrtest.c::all_tests[]`：清理重复条目（如旧 `autocorrelation`/`cumulative_sums` 通用入口），保证 27 项国密全部独立注册。
   - [ ] 更新 `TEST_COUNT`，并使用 `_Static_assert(sizeof(all_tests)/sizeof(all_tests[0]) == TEST_COUNT, ...)`（可选）。
   - [ ] 简化 `gmrtest_run_multiple` 派发逻辑：保留 poker/serial 多结果分支；其他完全使用 `get_test_by_id` 直接命中。

10. **Phase 9 配置与文档**
    - [ ] 更新 `src/main.c::gm_tests[]` 为 27 项。
    - [ ] 更新 `test/gm_config.ini` 默认值。
    - [ ] 更新 `Makefile`：新增源文件 + 依赖。
    - [ ] 更新 `todo.md` 进度（27/27 = 100%）。
    - [ ] 更新 `README.md` 与 `docs/add_test_guide.md` 附录清单。

11. **Phase 10 验证**
    - [ ] `make clean && make`：无错误、无警告。
    - [ ] `./bin/gmrtest -f test/random.bin -g`：所有 27 项均输出 p_value（在 [0,1]）、q_value、status。
    - [ ] `./bin/gmrtest -c test/gm_config.ini`：与 `-g` 等效。
    - [ ] `make pack && ./bin/gmrtest_packed -f test/random.bin -g`：位压缩模式同样正常。
    - [ ] `./bin/gmrtest -l`：列出 27 项国密项。

---

## 六、Verification（验证）

| 验证项 | 命令 / 检查 | 预期 |
|--------|-------------|------|
| 编译干净 | `make clean && make 2>&1 \| tee build.log` | exit 0，无 warning |
| 位压缩兼容 | `make pack` | 链接成功 |
| 国密预设 | `./bin/gmrtest -f test/random.bin -g` | 输出 27 项结果，无 ERROR |
| 配置文件 | `./bin/gmrtest -c test/gm_config.ini` | 与 `-g` 输出一致 |
| 列表展示 | `./bin/gmrtest -l` | 包含 27 项国密 ID |
| p 值合理性 | 检查输出 | 所有 p ∈ [0,1]，统计上 ≤2 项 FAILURE 在 α=0.01 下属正常 |
| 文档同步 | `grep -c "🆗" todo.md` | 应等于 27 |

---

## 七、Out of Scope（非本次范围）

- 非重叠模板匹配（NIST 项，国密不要求）
- 重叠模板匹配（NIST 项）
- 随机游程检测（NIST 项）
- 随机游程变体检测（NIST 项）

以上 4 项保持当前 stub 实现，不在本次实现范围内。
