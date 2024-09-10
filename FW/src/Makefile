# ISP & UART CONFIG
DEVICE       = cortex-m3
PORT         = /dev/ttyUSB0
BAUD         = 115200
ISP_CLK      = 12000
# FILES NAMES
MAIN_FILE    = main
HAL_FILE     = hal
TIMERS_FILE  = timers
PRINTF_FILE  = printf
MATH_FILE    = math

# COMPILER, LINKER
LINKER_SCRIPT = cmsis/stm32f103c8tx.ld -mthumb -Wl,-gc-sections -lc
GCC          = arm-none-eabi-gcc
AS           = arm-none-eabi-as
GPP          = arm-none-eabi-g++
LD           = arm-none-eabi-ld
OBJCP        = arm-none-eabi-objcopy

# COMPILATION OPTIONS DECLARATIONS
OPT          = -Wall -Os -std=c99 --specs=nosys.specs
OPT         += -mcpu=$(DEVICE) -mthumb -Wl,-Map,$(BUILD_DIR)output.map
# -WarningALL -OptimizationSize -standard=C99 -nostartfiles* -mthumb**

# * - The option -nostartfiles instructs the linker to not use the standard system startup
# functions nor link the code containing those functions.
# ** -  -mthumb means that it will generate thumb/T32 one)
DEF          = -DCORE_M3 -DENABLE_UNTESTED_CODE -DARMCM3 -DSTM32F10X
DEF         += -DUSE_OLD_STYLE_DATA_BSS_INIT

# DIRECTORIES
CMSIS_LIBS   = cmsis/
BUILD_DIR    = build/
TEST_DIR     = ../tests/

# APPEND EACH FILE TO SRC FOR COMPILATION
SRC          = $(MAIN_FILE).c
SRC         += $(HAL_FILE).c
SRC         += $(TIMERS_FILE).c
SRC         += $(PRINTF_FILE).c
SRC         += $(MATH_FILE).c
SRC         += $(CMSIS_LIBS)system_stm32f10x.c
SRCs        += $(CMSIS_LIBS)startup_stm32f103xb.s

# check if fw tests are going to be done
ifeq ($(MAKECMDGOALS),test)
	OPT     += -DTESTS
	SRC     += $(TEST_DIR)test.c $(TEST_DIR)test_functions.c
endif
# copy all $(SRC) files into $(OBJS) and rename them to .o
OBJS         = $(SRC:.c=.o)
OBJS        += $(SRCs:.s=.o)

# remove cmsis/ and ../tests/ from OBJS
SUB_DIRS_MCU = $(subst $(CMSIS_LIBS), , $(OBJS))
SUB_DIRS     = $(subst $(TEST_DIR), , $(SUB_DIRS_MCU))
# add prefix (BUILD_DIR) to each file from SUB_DIRS
TARGET_DIRS  = $(foreach objs, $(SUB_DIRS), $(addprefix $(BUILD_DIR), $(objs)))

all: dirs $(OBJS) $(MAIN_FILE).elf $(MAIN_FILE).bin readelf upload

test: dirs $(OBJS_TEST) $(OBJS) $(MAIN_FILE).elf $(MAIN_FILE).bin upload

dirs:
	mkdir -p $(BUILD_DIR)

%.o: %.c
	$(GCC) -c $(OPT) -I$(CMSIS_LIBS) -I$(TEST_DIR) $(DEF) $< -o $(BUILD_DIR)$(@F)

%.o: %.s
	$(AS) --fatal-warnings -mthumb -I$(CMSIS_LIBS) -I$(TEST_DIR) $< -o $(BUILD_DIR)$(@F)

%.elf: $(TARGET_DIRS)
	$(GCC) $(OPT) -g -T$(LINKER_SCRIPT) $(TARGET_DIRS) -o $(BUILD_DIR)$@

%bin: %elf
	$(OBJCP) -O binary -S $(BUILD_DIR)$< $(BUILD_DIR)$@

upload:
	sudo stm32flash -w $(BUILD_DIR)$(MAIN_FILE).bin -v $(PORT) -b $(BAUD) -m 8E1

uart:
	sudo screen $(PORT) $(BAUD)

readelf:
	readelf -a $(BUILD_DIR)$(MAIN_FILE).elf > $(BUILD_DIR)elf.txt

clean:
	rm -f $(BUILD_DIR)*

help:
	$(info Makefile for Cruise Control by CR)
	$(info Possible targets: all, test, upload, uart, readelf, clean, help)
