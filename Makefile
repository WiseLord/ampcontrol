AUDIOPROC = TDA7439
DISPLAY = KS0108

TUNER_LIST = RDA580X

# Lowercase argument
lc = $(shell echo $1 | tr A-Z a-z)

# Fimware file base name
TARG = ampcontrol_m16_$(call lc,$(AUDIOPROC))_$(call lc,$(DISPLAY))_$(call lc,$(TUNER_LIST))

MCU = atmega16
F_CPU = 16000000L

ifeq ($(AUDIOPROC), TDA7313)
  AUDIO_SRC = audio/tda7313.c
else ifeq ($(AUDIOPROC), TDA7318)
  AUDIO_SRC = audio/tda7318.c
else ifeq ($(AUDIOPROC), TDA7439)
  AUDIO_SRC = audio/tda7439.c
endif

FONTS = font-ks0066-ru-08.c font-ks0066-ru-24.c font-digits-32.c
ifeq ($(DISPLAY), KS0108)
  DISP_SRC = $(addprefix display/, ks0108.c $(FONTS))
else ifeq ($(DISPLAY), KS0066)
  DISP_SRC = display/ks0066.c
else ifeq ($(DISPLAY), LS020)
  DISP_SRC = $(addprefix display/, ls020.c $(FONTS))
else ifeq ($(DISPLAY), PCF8574)
  DISP_SRC = display/pcf8574.c
endif

SRCS = $(wildcard *.c) $(AUDIO_SRC) $(DISP_SRC)

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
  HARDWARE_RST = "YES"
endif
DEFINES += $(addprefix -D_, $(TUNER_LIST))

ifeq "$(findstring RDS, $(FEATURE_LIST))" "RDS"
  SRCS += tuner/rds.c
endif
DEFINES += $(addprefix -D_, $(FEATURE_LIST))

# Software SPI
ifeq "$(findstring YES, $(SOFTWARE_SPI))" "YES"
#  SRCS += spisw.c
  DEFINES += -D_SPISW
endif

ifeq "$(findstring YES, $(HARDWARE_RST))" "YES"
  DEFINES += -D_HARDWARE_RST
endif

# Build directory
BUILDDIR = build

OPTIMIZE = -Os -mcall-prologues -fshort-enums -ffunction-sections -fdata-sections -ffreestanding
DEBUG = -g -Wall -Werror
CFLAGS = $(DEBUG) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -MMD -MP -MT $(BUILDDIR)/$(*F).o -MF $(BUILDDIR)/$(*D)/$(*F).d
LDFLAGS = $(DEBUG) -mmcu=$(MCU) -Wl,--gc-sections -Wl,--relax

CC = avr-gcc
OBJCOPY = avr-objcopy

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT)

OBJS = $(addprefix $(BUILDDIR)/, $(SRCS:.c=.o))
ELF = $(BUILDDIR)/$(TARG).elf

all: $(ELF) size

$(ELF): $(OBJS)
	@mkdir -p $(addprefix $(BUILDDIR)/, $(SUBDIRS)) flash
	$(CC) $(LDFLAGS) -o $(ELF) $(OBJS) -lm
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram $(ELF) flash/$(TARG).hex

size:
	@sh ./size.sh $(ELF)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -D_atmega16 -D$(AUDIOPROC) -D$(DISPLAY) $(DEFINES) -c -o $@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: flash
flash: $(ELF)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:flash/$(TARG).hex:i

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0x3F:m -U hfuse:w:0xC1:m

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_en.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_ru.bin:r

eeprom_by:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_by.bin:r

eeprom_ua:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_ua.bin:r

# Other dependencies
-include $(OBJS:.o=.d)
