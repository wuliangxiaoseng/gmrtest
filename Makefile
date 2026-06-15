# gmrtest - 国密随机数检测工具 Makefile
# 
# 项目: gmrtest (GuoMi Random TEST)
# 描述: 实现国密规范要求的随机性统计检测算法

# ============================================================================
# 编译器配置
# ============================================================================

CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2
LDFLAGS ?= -lm

# 调试模式
ifdef DEBUG
    CFLAGS += -g -DDEBUG -O0
endif

# 位压缩模式 (BIT_PACKING=1)
# 默认不启用，使用字节/比特模式（兼容NIST原始实现）
# 启用后：8个比特压缩到1字节，节省87.5%内存，但访问稍慢
ifdef BIT_PACKING
    CFLAGS += -DBIT_PACKING=$(BIT_PACKING)
    TARGET_SUFFIX := _packed
endif

# ============================================================================
# 目录配置
# ============================================================================

SRC_DIR     = src
INC_DIR     = include
BUILD_DIR   = build
BIN_DIR     = bin

# ============================================================================
# 目标配置
# ============================================================================

TARGET      = $(BIN_DIR)/gmrtest$(TARGET_SUFFIX)

# 源文件
SRCS_MAIN   = $(SRC_DIR)/main.c \
              $(SRC_DIR)/gmrtest.c

SRCS_TESTS  = $(SRC_DIR)/tests/frequency.c \
              $(SRC_DIR)/tests/block_frequency.c \
              $(SRC_DIR)/tests/poker.c \
              $(SRC_DIR)/tests/runs.c \
              $(SRC_DIR)/tests/serial.c \
              $(SRC_DIR)/tests/runs_distribution.c \
              $(SRC_DIR)/tests/longest_run.c \
              $(SRC_DIR)/tests/binary_derivative.c \
              $(SRC_DIR)/tests/cumulative_sums.c \
              $(SRC_DIR)/tests/self_correlation.c \
              $(SRC_DIR)/tests/approximate_entropy.c \
              $(SRC_DIR)/tests/rank.c \
              $(SRC_DIR)/tests/dft.c \
              $(SRC_DIR)/tests/universal.c \
              $(SRC_DIR)/tests/linear_complexity.c \
              $(SRC_DIR)/tests/tests_stub.c

SRCS_UTILS  = $(SRC_DIR)/utils/math.c \
              $(SRC_DIR)/utils/io.c \
              $(SRC_DIR)/utils/matrix.c \
              $(SRC_DIR)/utils/fft.c

SRCS        = $(SRCS_MAIN) $(SRCS_TESTS) $(SRCS_UTILS)

# 目标文件
OBJS        = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# ============================================================================
# 包含路径
# ============================================================================

INCLUDES    = -I$(INC_DIR)

# ============================================================================
# 构建规则
# ============================================================================

.PHONY: all clean debug install uninstall test help pack

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "构建完成: $@"
	@echo ""
	@echo "存储模式: $(if $(BIT_PACKING),位压缩模式 (节省87.5%内存),字节/比特模式 (兼容NIST原始实现))"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ============================================================================
# 位压缩模式构建
# ============================================================================

pack:
	$(MAKE) BIT_PACKING=1

# ============================================================================
# 清理规则
# ============================================================================

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "清理完成"

# ============================================================================
# 调试构建
# ============================================================================

debug:
	$(MAKE) DEBUG=1

debug-pack:
	$(MAKE) DEBUG=1 BIT_PACKING=1

# ============================================================================
# 安装规则
# ============================================================================

PREFIX ?= /usr/local

install: $(TARGET)
	install -d $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin/
	install -d $(PREFIX)/include/gmrtest
	install -m 644 $(INC_DIR)/*.h $(PREFIX)/include/gmrtest/
	@echo "安装完成: $(PREFIX)/bin/gmrtest"

uninstall:
	rm -f $(PREFIX)/bin/gmrtest
	rm -rf $(PREFIX)/include/gmrtest
	@echo "卸载完成"

# ============================================================================
# 测试规则
# ============================================================================

test: $(TARGET)
	@echo "运行测试..."
	@./$(TARGET) --version
	@./$(TARGET) --list

# ============================================================================
# 帮助信息
# ============================================================================

help:
	@echo "gmrtest Makefile 使用说明"
	@echo ""
	@echo "构建目标:"
	@echo "  all          - 构建项目 (默认, 字节/比特模式)"
	@echo "  pack         - 位压缩模式构建 (节省内存)"
	@echo "  clean        - 清理构建文件"
	@echo "  debug        - 调试模式构建"
	@echo "  debug-pack   - 调试+位压缩模式"
	@echo "  install      - 安装到系统 (需要root权限)"
	@echo "  uninstall    - 从系统卸载"
	@echo "  test         - 运行测试"
	@echo "  help         - 显示此帮助"
	@echo ""
	@echo "变量:"
	@echo "  PREFIX       - 安装路径前缀 (默认: /usr/local)"
	@echo "  CC           - 编译器 (默认: gcc)"
	@echo "  CFLAGS       - 编译选项"
	@echo "  DEBUG=1      - 启用调试模式"
	@echo "  BIT_PACKING=1 - 启用位压缩模式"
	@echo ""
	@echo "存储模式说明:"
	@echo "  字节/比特模式 (默认): 每个比特占用1字节，访问速度快，兼容NIST原始实现"
	@echo "  位压缩模式 (pack):   8个比特压缩到1字节，节省87.5%内存，适合大数据量"
	@echo ""
	@echo "示例:"
	@echo "  make              # 构建项目 (字节/比特模式)"
	@echo "  make pack         # 位压缩模式构建"
	@echo "  make DEBUG=1      # 调试模式构建"
	@echo "  make clean        # 清理"
	@echo "  sudo make install # 安装"

# ============================================================================
# 依赖关系
# ============================================================================

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/gmrtest.h
$(BUILD_DIR)/gmrtest.o: $(SRC_DIR)/gmrtest.c $(INC_DIR)/gmrtest.h $(INC_DIR)/tests.h
$(BUILD_DIR)/tests/frequency.o: $(SRC_DIR)/tests/frequency.c $(INC_DIR)/tests.h $(INC_DIR)/utils.h
$(BUILD_DIR)/tests/tests_stub.o: $(SRC_DIR)/tests/tests_stub.c $(INC_DIR)/tests.h $(INC_DIR)/utils.h
$(BUILD_DIR)/utils/math.o: $(SRC_DIR)/utils/math.c $(INC_DIR)/utils.h
$(BUILD_DIR)/utils/io.o: $(SRC_DIR)/utils/io.c $(INC_DIR)/utils.h
