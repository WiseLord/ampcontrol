AUDIOPROC = TDA7439
DISPLAY = KS0108
TUNER = TEA5767

# Lowercase argument
lc = $(shell echo $1 | tr A-Z a-z)

# Fimware file base name
TARG = ampcontrol_$(call lc,$(AUDIOPROC))_$(call lc,$(DISPLAY))_$(call lc,$(TUNER))
# EEPROM file base name
EEPROM = eeprom_$(call lc,$(AUDIOPROC))

SPECT_SRC = fft.c adc.c
CTRL_SRC = input.c rc5.c
TUNER_SRC = tuner/tea5767.c

FONTS = font-ks0066-ru-08.c font-ks0066-ru-24.c font-digits-32.c
ifeq ($(DISPLAY), KS0108)
  DISP_SRC = $(addprefix display/, ks0108.c $(FONTS))
else ifeq ($(DISPLAY), KS0066)
  DISP_SRC = display/ks0066.c
else ifeq ($(DISPLAY), LS020)
DISP_SRC = $(addprefix display/, ls020.c $(FONTS))
endif

ifeq ($(TUNER), TEA5767)
  TUNER_SRC = tuner/tea5767.c
else ifeq ($(TUNER), TUX032)
TUNER_SRC = tuner/tux032.c
endif

SRCS_CONST = eeprom.c i2c.c ds1307.c $(SPECT_SRC) $(CTRL_SRC) $(DISP_SRC) $(TUNER_SRC)
SRCS_VAR = main.c audio.c display.c tuner.c

MCU = atmega16
F_CPU = 16000000L

CS = -fexec-charset=ks0066-ru

OPTIMIZE = -Os -mcall-prologues
CFLAGS = -g -Wall -Werror -lm $(OPTIMIZE) $(CS) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS = -g -Wall -Werror -mmcu=$(MCU)

CC = avr-gcc
OBJCOPY = avr-objcopy

AVRDUDE = avrdude
AD_MCU = -p m16
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper
#AD_PROG = -c usbasp
#AD_PORT = -P usbasp

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT)

OBJS_CONST = $(SRCS_CONST:.c=.o)
OBJS_VAR = $(SRCS_VAR:.c=.o)
OBJS = $(OBJS_CONST) $(OBJS_VAR)

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf $(OBJS) -lm
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $@.elf flash/$@.hex
	$(OBJCOPY) -O binary -R .eeprom -R .nwram  $@.elf $@.bin
	echo; wc -c $@.bin; echo; rm -f $@.bin $@.elf

%.o: %.c
	$(CC) $(CFLAGS) -D$(AUDIOPROC) -D$(DISPLAY) -D$(TUNER) -c -o $@ $<

clean_var:
	rm -f $(OBJS_VAR)

clean_const:
	rm -f $(OBJS_CONST)

clean: clean_var clean_const

flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U flash:w:flash/$(TARG).hex:i

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0xff:m -U hfuse:w:0xc1:m

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U eeprom:w:eeprom/$(EEPROM)_en.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U eeprom:w:eeprom/$(EEPROM)_ru.bin:r
