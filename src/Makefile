DISPLAY = ST7920

MCU = atmega32
F_CPU = 16000000L

APROC_LIST = TDA7439 TDA731X TDA7448 PT232X TEA63X0 TUNER_AUDIO
TUNER_LIST = TEA5767 RDA580X TUX032 SI470X
FEATURE_LIST = ALARM UARTCONTROL RDS RDA5807_DF

ifeq ($(MCU), atmega32)
APROC_LIST += PGA2310
#APROC_LIST += R2S15904SP
TUNER_LIST += LM7001 LC72131
FEATURE_LIST += TEMPCONTROL
endif

TARG = ampcontrol_$(MCU)_$(shell echo $(DISPLAY) | tr A-Z a-z)

SRCS = main.c
SRCS += input.c actions.c i2c.c rtc.c remote.c fft.c adc.c

DEFINES = -D_$(MCU)

# Display source files
FONTS_SRC = $(wildcard display/font*.c)
ICONS_SRC = $(wildcard display/icon*.c)
ifeq "$(DISPLAY)" "KS0066_16X2_8BIT"
  SRCS += display/ks0066.c
else ifeq "$(DISPLAY)" "KS0066_16X2_PCF8574"
  SRCS += display/ks0066.c
else ifeq "$(DISPLAY)" "LS020"
  SRCS += display/ls020.c $(FONTS_SRC) $(ICONS_SRC)
else ifeq "$(DISPLAY)" "ST7920"
  SRCS += display/gdfb.c display/st7920.c $(FONTS_SRC) $(ICONS_SRC)
else ifeq "$(DISPLAY)" "SSD1306"
  SRCS += display/gdfb.c display/ssd1306.c $(FONTS_SRC) $(ICONS_SRC)
else ifeq "$(DISPLAY)" "SH1106"
  SRCS += display/gdfb.c display/ssd1306.c $(FONTS_SRC) $(ICONS_SRC)
  DEFINES += -D_SSD1306
else
  SRCS += display/gdfb.c display/ks0108.c $(FONTS_SRC) $(ICONS_SRC)
endif
SRCS += display.c
DEFINES += -D_$(DISPLAY)

# Audio source files
SRCS += audio/audio.c
ifeq "$(findstring TDA7439, $(APROC_LIST))" "TDA7439"
  SRCS += audio/tda7439.c
endif
ifeq "$(findstring TDA731X, $(APROC_LIST))" "TDA731X"
  SRCS += audio/tda731x.c
endif
ifeq "$(findstring TDA7448, $(APROC_LIST))" "TDA7448"
  SRCS += audio/tda7448.c
endif
ifeq "$(findstring PT232X, $(APROC_LIST))" "PT232X"
  SRCS += audio/pt232x.c
endif
ifeq "$(findstring TEA63X0, $(APROC_LIST))" "TEA63X0"
  SRCS += audio/tea63x0.c
endif
ifeq "$(findstring PGA2310, $(APROC_LIST))" "PGA2310"
  SRCS += audio/pga2310.c
  SOFTWARE_SPI = YES
endif
ifeq "$(findstring R2S15904SP, $(APROC_LIST))" "R2S15904SP"
  SRCS += audio/r2s15904sp.c
  SOFTWARE_SPI = YES
endif
DEFINES += $(addprefix -D_, $(APROC_LIST))

# Tuner source files
SRCS += tuner/tuner.c
ifeq "$(findstring TEA5767, $(TUNER_LIST))" "TEA5767"
  SRCS += tuner/tea5767.c
endif
ifeq "$(findstring RDA580X, $(TUNER_LIST))" "RDA580X"
  SRCS += tuner/rda580x.c
endif
ifeq "$(findstring TUX032, $(TUNER_LIST))" "TUX032"
  SRCS += tuner/tux032.c
endif
ifeq "$(findstring LM7001, $(TUNER_LIST))" "LM7001"
  SRCS += tuner/lm7001.c
  SOFTWARE_SPI = YES
endif
ifeq "$(findstring LC72131, $(TUNER_LIST))" "LC72131"
  SRCS += tuner/lc72131.c
  SOFTWARE_SPI = YES
endif
ifeq "$(findstring SI470X, $(TUNER_LIST))" "SI470X"
  SRCS += tuner/si470x.c
endif
DEFINES += $(addprefix -D_, $(TUNER_LIST))

# Features
ifeq "$(findstring TEMPCONTROL, $(FEATURE_LIST))" "TEMPCONTROL"
  SRCS += temp.c ds18x20.c
endif
ifeq "$(findstring UARTCONTROL, $(FEATURE_LIST))" "UARTCONTROL"
  SRCS += uart.c
endif
ifeq "$(findstring RDS, $(FEATURE_LIST))" "RDS"
  SRCS += tuner/rds.c
endif
ifeq "$(findstring ALARM, $(FEATURE_LIST))" "ALARM"
  SRCS += alarm.c
endif
DEFINES += $(addprefix -D_, $(FEATURE_LIST))

# Software SPI
ifeq "$(findstring YES, $(SOFTWARE_SPI))" "YES"
  SRCS += spisw.c
  DEFINES += -D_SPISW
endif

# Build directory
BUILDDIR = build

OPTIMIZE = -Os -mcall-prologues -fshort-enums -ffunction-sections -fdata-sections -ffreestanding -flto
WARNLEVEL = -Wall -Werror
CFLAGS = $(WARNLEVEL) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -MMD -MP -MT $(BUILDDIR)/$(*F).o -MF $(BUILDDIR)/$(*D)/$(*F).d
LDFLAGS = $(WARNLEVEL) -mmcu=$(MCU) -Wl,--gc-sections -Wl,--relax

# Main definitions

PREFIX = avr-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
OD = $(PREFIX)objdump
AR = $(PREFIX)ar
SZ = $(PREFIX)size

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT) -V -B 0.5

SUBDIRS = audio display tuner

OBJS = $(addprefix $(BUILDDIR)/, $(SRCS:.c=.o))
ELF = $(BUILDDIR)/$(TARG).elf
HEX = flash/$(TARG).hex

all: $(HEX) size

$(HEX): $(ELF)
	$(CP) -O ihex -R .eeprom -R .nwram $(ELF) $(HEX)

$(ELF): $(OBJS)
	@mkdir -p $(addprefix $(BUILDDIR)/, $(SUBDIRS)) flash
	$(CC) $(LDFLAGS) -o $(ELF) $(OBJS)
	$(OD) -h -S $(ELF) > $(BUILDDIR)/$(TARG).lss

size: $(ELF)
	$(SZ) $(ELF)

$(BUILDDIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: flash
flash:  $(HEX)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:flash/$(TARG).hex:i

fuse:
ifeq ($(MCU), atmega32)
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0x3F:m -U hfuse:w:0xC1:m
else
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0xff:m -U hfuse:w:0xd1:m -U efuse:w:0xfc:m
endif

eeprom_by:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:../eeprom/eeprom_by.bin:r

eeprom_bg:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:../eeprom/eeprom_bg.bin:r

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:../eeprom/eeprom_en.bin:r

eeprom_fr:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:../eeprom/eeprom_fr.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:../eeprom/eeprom_ru.bin:r

eeprom_ua:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:../eeprom/eeprom_ua.bin:r

# Other dependencies
-include $(OBJS:.o=.d)
