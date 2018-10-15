TOP = ../..
include $(TOP)/mk/common.mk

OUTPUT = pmsynth

# hal sources
HAL_DIR = $(TOP)/soc/st/stm32f4/hal/src
SRC += $(HAL_DIR)/stm32f4xx_hal.c \
	$(HAL_DIR)/stm32f4xx_hal_rcc.c \
	$(HAL_DIR)/stm32f4xx_hal_rcc_ex.c \
	$(HAL_DIR)/stm32f4xx_hal_cortex.c \
	$(HAL_DIR)/stm32f4xx_hal_gpio.c \
	$(TOP)/soc/st/stm32f4/system_stm32f4xx.c \

# soc library sources
LIB_DIR = $(TOP)/soc/st/stm32f4/lib
SRC += $(LIB_DIR)/gpio.c \
	$(LIB_DIR)/delay.c \
	$(LIB_DIR)/i2c.c \
	$(LIB_DIR)/i2s.c \
	$(LIB_DIR)/dma.c \
	$(LIB_DIR)/adc.c \
	$(LIB_DIR)/usart.c \
	$(LIB_DIR)/rng.c \
	$(LIB_DIR)/spi.c \

# target sources
TARGET_DIR = $(TOP)/target/mb997
SRC += $(TARGET_DIR)/main.c \
	$(TARGET_DIR)/audio.c \
	$(TARGET_DIR)/display.c \

# common
COMMON_DIR = $(TOP)/common
SRC += $(COMMON_DIR)/logging.c \
	$(COMMON_DIR)/debounce.c \
	$(COMMON_DIR)/syscalls.c \
	$(COMMON_DIR)/rand.c \
	$(COMMON_DIR)/rtt/SEGGER_RTT.c \
	$(COMMON_DIR)/rtt/SEGGER_RTT_printf.c \

# drivers
DRIVERS_DIR = $(TOP)/drivers
SRC += $(DRIVERS_DIR)/cs43l22.c \
	$(DRIVERS_DIR)/ili9341.c \

# synth files
SYNTH_DIR = $(TOP)/pmsynth
SRC += $(SYNTH_DIR)/sin.c \
	$(SYNTH_DIR)/midi.c \
	$(SYNTH_DIR)/seq.c \
	$(SYNTH_DIR)/pmsynth.c \
	$(SYNTH_DIR)/event.c \
	$(SYNTH_DIR)/adsr.c \
	$(SYNTH_DIR)/pan.c \
	$(SYNTH_DIR)/ks.c \
	$(SYNTH_DIR)/lpf.c \
	$(SYNTH_DIR)/noise.c \
	$(SYNTH_DIR)/block.c \
	$(SYNTH_DIR)/pow.c \
	$(SYNTH_DIR)/bmark.c \
	$(SYNTH_DIR)/patch2.c \
	$(SYNTH_DIR)/patch4.c \
	$(SYNTH_DIR)/patch6.c \
	$(SYNTH_DIR)/patch7.c \
	$(SYNTH_DIR)/waveguide.c \
	$(SYNTH_DIR)/patch8.c \
	$(SYNTH_DIR)/waveguide2d.c \
	$(SYNTH_DIR)/patch9.c \
	$(SYNTH_DIR)/woodwind.c \
	$(SYNTH_DIR)/handler.c \
	$(SYNTH_DIR)/patch10.c \
	$(SYNTH_DIR)/waveguidebanded.c \

# ui
UI_DIR = $(TOP)/ui
SRC += $(UI_DIR)/fonts.c \
	$(UI_DIR)/term.c \
	$(UI_DIR)/graphics.c \
	$(UI_DIR)/profont22.c \

OBJ = $(patsubst %.c, %.o, $(SRC))
OBJ += $(TARGET_DIR)/start.o
OBJ += $(SYNTH_DIR)/p4wave.o

# include files

# include paths
INCLUDE += -I$(TOP)/soc/st/stm32f4/lib
INCLUDE += -I$(TOP)/soc/st/stm32f4/hal/inc
INCLUDE += -I$(TOP)/soc/st/stm32f4/cmsis
INCLUDE += -I$(DRIVERS_DIR)
INCLUDE += -I$(TARGET_DIR)
INCLUDE += -I$(COMMON_DIR)
INCLUDE += -I$(COMMON_DIR)/rtt
INCLUDE += -I$(SYNTH_DIR)
INCLUDE += -I$(UI_DIR)

# defines
DEFINE = -DSTM32F407xx
DEFINE += -DSTDIO_RTT

# linker flags
LDSCRIPT = stm32f407vg_flash.ld
X_LDFLAGS = -T$(LDSCRIPT) -Wl,-Map,$(OUTPUT).map -Wl,--gc-sections

# unused parameters in the HAL code
X_CFLAGS += -Wno-unused-parameter
X_CFLAGS += -Wno-strict-prototypes

.S.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@
.c.o:
	$(X_GCC) $(INCLUDE) $(DEFINE) $(X_CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(OBJ)
	$(X_GCC) $(X_CFLAGS) $(X_LDFLAGS) $(OBJ) -lm -o $(OUTPUT)
	$(X_OBJCOPY) -O binary $(OUTPUT) $(OUTPUT).bin

clean:
	-rm $(OBJ)	
	-rm $(OUTPUT)
	-rm $(OUTPUT).map	
	-rm $(OUTPUT).bin	
