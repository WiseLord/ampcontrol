AUDIOPROC = TDA7439
DISPLAY = ST7920
TUNER = TEA5767

# Lowercase argument
lc = $(shell echo $1 | tr A-Z a-z)

# Fimware file base name
TARG = ampcontrol_m32fb_$(call lc,$(AUDIOPROC))_$(call lc,$(DISPLAY))_$(call lc,$(TUNER))

SPECT_SRC = fft.c adc.c
CTRL_SRC = input.c rc5.c

ifeq ($(AUDIOPROC), TDA7313)
  AUDIO_SRC = audio/tda7313.c
else ifeq ($(AUDIOPROC), TDA7318)
  AUDIO_SRC = audio/tda7318.c
else ifeq ($(AUDIOPROC), TDA7439)
  AUDIO_SRC = audio/tda7439.c
endif

FONTS = font-ks0066-ru-08.c font-ks0066-ru-24.c font-digits-32.c
ifeq ($(DISPLAY), ST7920)
  DISP_SRC = display.c $(addprefix display/, gdfb.c st7920.c $(FONTS))
else ifeq ($(DISPLAY), KS0108A)
  DISP_SRC = display.c $(addprefix display/, gdfb.c ks0108.c $(FONTS))
else ifeq ($(DISPLAY), KS0108B)
  DISP_SRC = display.c $(addprefix display/, gdfb.c ks0108.c $(FONTS))
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

ifneq ($(TUNER), LM7001)
TEMP_SRC = ds18x20.c temp.c
endif

SRCS = main.c eeprom.c i2c.c ds1307.c $(SPECT_SRC) $(CTRL_SRC) $(AUDIO_SRC) $(DISP_SRC) $(TUNER_SRC) $(TEMP_SRC)

MCU = atmega32
F_CPU = 16000000L

OPTIMIZE = -O2 -mcall-prologues -fshort-enums
DEBUG = -g -Wall -Werror
CFLAGS = $(DEBUG) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS = $(DEBUG) -mmcu=$(MCU)

CC = avr-gcc
OBJCOPY = avr-objcopy

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT) -V

OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
ELF = $(OBJDIR)/$(TARG).elf

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(ELF) $(OBJS) -lm
	mkdir -p flash
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram $(ELF) flash/$@.hex
	./size.sh $(ELF)

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -D$(AUDIOPROC) -D$(DISPLAY) -D$(TUNER) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:flash/$(TARG).hex:i

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0xff:m -U hfuse:w:0xc1:m

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_en.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_ru.bin:r

eeprom_by:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_by.bin:r

eeprom_ua:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/eeprom_ua.bin:r
