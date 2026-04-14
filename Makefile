# --- 静默控制逻辑 ---
ifeq ($(Q),1)
  M = @
else
  M = 
endif

# --- 项目设置 ---
TARGET    = gd32e230_std
BUILD_DIR = build

# --- 工具链 ---
PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
CXX     := $(PREFIX)g++
AS      := $(PREFIX)gcc -x assembler-with-cpp
CP      := $(PREFIX)objcopy
SZ      := $(PREFIX)size
LD      := $(CXX)

# --- 路径定义 ---
FW_DIR    = firmware
STD_DIR   = $(FW_DIR)/GD32E23x_standard_peripheral
CMSIS_DIR = $(FW_DIR)/cmsis

C_DEFS = -DGD32E230 -DUSE_STDPERIPH_DRIVER
C_INCLUDES = \
	-Iinc \
	-I$(CMSIS_DIR)/inc \
	-I$(STD_DIR)/Include \
	-Iapps 

CPP_INCLUDES = $(C_INCLUDES) \
	-Idrivers \
	-Icommon

# --- 源文件扫描 ---
# C_SOURCES
C_SOURCES = \
	$(CMSIS_DIR)/src/system_gd32e23x.c \
	$(CMSIS_DIR)/src/syscalls.c \
	$(wildcard $(STD_DIR)/Source/*.c) \
	$(wildcard apps/*.c) \
	$(wildcard common/*.c)
# CXX_SOURCES
CXX_SOURCES = \
	$(wildcard apps/*.cpp) \
	$(wildcard drivers/*.cpp) \
	$(wildcard common/*.cpp)
# ASM_SOURCES
ASM_SOURCES = gcc_startup/startup_gd32e23x.S

# --- 编译参数 ---
CPU  = -mcpu=cortex-m23
MCU  = $(CPU) -mthumb -mfloat-abi=soft

# --- 编译选项 ---
OPT = -Og
DEBUG_FLAGS = -g -gdwarf-2

COMMON_FLAGS = $(MCU) $(OPT) $(DEBUG_FLAGS) -Wall -fdata-sections -ffunction-sections
DEP_FLAGS    = -MMD -MP -MF"$(@:%.o=%.d)"

CFLAGS   = $(COMMON_FLAGS) $(C_DEFS) $(C_INCLUDES) $(DEP_FLAGS)
CXXFLAGS = $(COMMON_FLAGS) $(C_DEFS) $(CPP_INCLUDES) $(DEP_FLAGS) \
           -fno-exceptions -fno-rtti -fno-threadsafe-statics -std=c++17 \
		   -Wno-register
AS_FLAGS = $(MCU) $(OPT)

# --- 链接配置 ---
LDSCRIPT = ldscripts/gd32e23x_flash.ld
LDFLAGS  = $(MCU) -specs=nano.specs -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--gc-sections

# --- 对象文件处理 ---
# 使用 patsubst 确保路径替换极其精确
OBJECTS  = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
OBJECTS += $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CXX_SOURCES))
OBJECTS += $(patsubst %.S, $(BUILD_DIR)/%.o, $(ASM_SOURCES))

# 提取所有子目录层级
OBJ_DIRS = $(sort $(dir $(OBJECTS)))

# --- 编译规则 ---
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

# 链接 ELF
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@echo "Linking..."
	$(M)$(LD) $(OBJECTS) $(LDFLAGS) -o $@
	$(M)$(SZ) $@

# 规则：编译 C (确保使用 mkdir -p)
$(BUILD_DIR)/%.o: %.c
	@echo "compiling C file: $<"
	@if not exist "$(dir $@)" mkdir "$(subst /,\,$(dir $@))"
	$(if $(M),@echo "CC  $<")
	$(M)$(CC) -c $(CFLAGS) $< -o $@

# 规则：编译 C++
$(BUILD_DIR)/%.o: %.cpp
	@echo "compiling C++ file: $<"
	@if not exist "$(dir $@)" mkdir "$(subst /,\,$(dir $@))"
	$(if $(M),@echo "CXX $<")
	$(M)$(CXX) -c $(CXXFLAGS) $< -o $@

# 规则：编译 汇编
$(BUILD_DIR)/%.o: %.S
	@echo "compiling ASM file: $<"
	@if not exist "$(dir $@)" mkdir "$(subst /,\,$(dir $@))"
	$(if $(M),@echo "AS  $<")
	$(M)$(AS) -c $(AS_FLAGS) $< -o $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(if $(M),@echo "Generating bin: $@")
	$(M)$(CP) -O binary $< $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(if $(M),@echo "Generating hex: $@")
	$(M)$(CP) -O ihex $< $@

clean:
	@echo "Cleaning..."
ifeq ($(OS),Windows_NT)
	$(M)rmdir /S /Q $(BUILD_DIR)
else
	$(M)rm -rf $(BUILD_DIR)
endif

flash: all
# 	openocd -f openocd_gdlink/openocd_gdlink.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"
	openocd \
		-f interface/cmsis-dap.cfg \
		-f target/gd32e23x.cfg \
		-c "init" \
		-c "reset init" \
		-c "flash write_image erase $(BUILD_DIR)/$(TARGET).bin 0x08000000 bin" \
		-c "reset run" \
		-c "exit"

.PHONY: all clean flash

# 包含依赖文件
-include $(wildcard $(BUILD_DIR)/**/*.d)