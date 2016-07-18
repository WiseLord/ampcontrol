AUDIOPROC = TDA7313
TUNER = RDA580X
WIRE = 4BIT

# Lowercase argument
lc = $(shell echo $1 | tr A-Z a-z)
# Fimware file base name
TARG = ampcontrol_m8_$(call lc,$(WIRE))_$(call lc,$(AUDIOPROC))_$(call lc,$(TUNER))

MCU = atmega8
F_CPU = 8000000L

ifeq ($(AUDIOPROC), TDA7313)
  AUDIO_SRC = audio/tda7313.c
else ifeq ($(AUDIOPROC), TDA7318)
  AUDIO_SRC = audio/tda7318.c
else ifeq ($(AUDIOPROC), TDA7439)
  AUDIO_SRC = audio/tda7439.c
endif

TUNER_SRC = tuner/tuner.c
ifeq ($(TUNER), TEA5767)
  TUNER_SRC += tuner/tea5767.c
else ifeq ($(TUNER), TUX032)
  TUNER_SRC += tuner/tux032.c
else ifeq ($(TUNER), LM7001)
  TUNER_SRC += tuner/lm7001.c
else ifeq ($(TUNER), RDA580X)
  TUNER_SRC += tuner/rda580x.c
endif

SRCS = $(wildcard *.c) $(AUDIO_SRC) $(TUNER_SRC)

# Build directory
BUILDDIR = build

OPTIMIZE = -Os -mcall-prologues -fshort-enums -ffunction-sections -fdata-sections
DEBUG = -g -Wall -Werror
CFLAGS = $(DEBUG) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -MMD -MP -MT $(BUILDDIR)/$(*F).o -MF $(BUILDDIR)/$(*D)/$(*F).d
LDFLAGS = $(DEBUG) -mmcu=$(MCU) -Wl,-gc-sections

# Main definitions
DEFINES  += -D$(AUDIOPROC) -DKS0066_WIRE_$(WIRE)
DEFINES += -D_$(TUNER)

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
HEX = flash/$(TARG).hex

all: $(HEX) size

$(ELF): $(OBJS)
	@mkdir -p $(addprefix $(BUILDDIR)/, $(SUBDIRS)) flash
	$(CC) $(LDFLAGS) -o $(ELF) $(OBJS) -lm
	$(OBJDUMP) -h -S $(ELF) > $(BUILDDIR)/$(TARG).lss

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram $(ELF) flash/$(TARG).hex

size:
	@sh ./size.sh $(ELF)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: flash
flash: $(HEX)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:$(HEX):i

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

# Other dependencies
-include $(OBJS:.o=.d)
