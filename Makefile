AUDIOPROC = TDA7313
TUNER = TEA5767
WIRE = 4BIT

# Lowercase argument
lc = $(shell echo $1 | tr A-Z a-z)
# Fimware file base name
TARG = ampcontrol_m8_$(call lc,$(WIRE))_$(call lc,$(AUDIOPROC))_$(call lc,$(TUNER))

MCU = atmega8
F_CPU = 8000000L

SPECT_SRC = fft.c adc.c
CTRL_SRC = input.c rc5.c
DISP_SRC = display.c display/ks0066.c

ifeq ($(AUDIOPROC), TDA7313)
  AUDIO_SRC = audio/tda7313.c
else ifeq ($(AUDIOPROC), TDA7318)
  AUDIO_SRC = audio/tda7318.c
else ifeq ($(AUDIOPROC), TDA7439)
  AUDIO_SRC = audio/tda7439.c
endif

ifeq ($(TUNER), TEA5767)
  TUNER_SRC = tuner.c tuner/tea5767.c
else ifeq ($(TUNER), TUX032)
  TUNER_SRC = tuner.c tuner/tux032.c
else ifeq ($(TUNER), LM7001)
  TUNER_SRC = tuner.c tuner/lm7001.c
else ifeq ($(TUNER), RDA5807)
  TUNER_SRC = tuner.c tuner/rda5807.c
endif

SRCS = main.c i2c.c ds1307.c $(SPECT_SRC) $(CTRL_SRC) $(AUDIO_SRC) $(DISP_SRC) $(TUNER_SRC)

# Build directory
BUILDDIR = build

OPTIMIZE = -Os -mcall-prologues -fshort-enums -ffunction-sections -fdata-sections
DEBUG = -g -Wall -Werror
CFLAGS = $(DEBUG) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -MMD -MP -MT $(BUILDDIR)/$(*F).o -MF $(BUILDDIR)/$(*D)/$(*F).d
LDFLAGS = $(DEBUG) -mmcu=$(MCU) -Wl,-gc-sections

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT)

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
	$(CC) $(CFLAGS) -D$(AUDIOPROC) -D$(TUNER) -DKS0066_WIRE_$(WIRE) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

.PHONY: flash
flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:flash/$(TARG).hex:i

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0x24:m -U hfuse:w:0xC1:m

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_en.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_ru.bin:r

eeprom_by:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_by.bin:r

eeprom_ua:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_ua.bin:r
