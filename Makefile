DISPLAY = KS0108A

TARG = ampcontrol_m32_$(shell echo $(DISPLAY) | tr A-Z a-z)

MCU = atmega32
F_CPU = 16000000L

AUDIO_SRC = $(wildcard audio/*.c)
TUNER_SRC = $(wildcard tuner/*.c)

FONTS_SRC = $(wildcard display/font*.c)
ICONS_SRC = $(wildcard display/icon*.c)

ifeq ($(DISPLAY), KS0066_16X2)
  DISP_SRC = display/ks0066.c
else ifeq ($(DISPLAY), KS0066_16X2_PCF8574)
  DISP_SRC = display/ks0066.c
else ifeq ($(DISPLAY), LS020)
  DISP_SRC = display/ls020.c $(FONTS_SRC) $(ICONS_SRC)
else ifeq ($(DISPLAY), ST7920)
  DISP_SRC = display/gdfb.c display/st7920.c $(FONTS_SRC) $(ICONS_SRC)
else ifeq ($(DISPLAY), SSD1306)
  DISP_SRC = display/gdfb.c display/ssd1306.c $(FONTS_SRC) $(ICONS_SRC)
else
  DISP_SRC = display/gdfb.c display/ks0108.c $(FONTS_SRC) $(ICONS_SRC)
endif

SRCS = $(wildcard *.c) $(AUDIO_SRC) $(TUNER_SRC) $(DISP_SRC)

# Build directory
BUILDDIR = build

OPTIMIZE = -Os -mcall-prologues -fshort-enums -ffunction-sections -fdata-sections -ffreestanding
DEBUG = -g -Wall -Werror
CFLAGS = $(DEBUG) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -MMD -MP -MT $(BUILDDIR)/$(*F).o -MF $(BUILDDIR)/$(*D)/$(*F).d
LDFLAGS = $(DEBUG) -mmcu=$(MCU) -Wl,--gc-sections -Wl,--relax

# Main definitions
DEFINES  += -D$(DISPLAY)
# Supported tuners
DEFINES += -D_TEA5767 -D_RDA580X -D_TUX032 -D_LM7001 -D_LC72131 -D_RDS
# Supported audioprocessors
DEFINES += -D_TDA7439 -D_TDA731X -D_TDA7448 -D_PT232X -D_TEA63X0 -D_PGA2310 -D_RDA580X_AUDIO

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT) -V -B 0.5

SUBDIRS = audio display tuner

OBJS = $(addprefix $(BUILDDIR)/, $(SRCS:.c=.o))
ELF = $(BUILDDIR)/$(TARG).elf

all: $(ELF) size

$(ELF): $(OBJS)
	@mkdir -p $(addprefix $(BUILDDIR)/, $(SUBDIRS)) flash
	$(CC) $(LDFLAGS) -o $(ELF) $(OBJS) -lm
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram $(ELF) flash/$(TARG).hex
	$(OBJDUMP) -h -S $(ELF) > $(BUILDDIR)/$(TARG).lss

size:
	@sh ./size.sh $(ELF)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

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
