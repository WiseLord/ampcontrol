AUDIOPROC = TDA7439
DISPLAY = KS0108

TARG = ampcontrol_$(shell echo $(AUDIOPROC) | tr A-Z a-z)_$(shell echo $(DISPLAY) | tr A-Z a-z)

SPECT_SRC = fft.c adc.c
CTRL_SRC = input.c rc5.c
FM_SRC = tea5767.c

ifeq ($(DISPLAY), KS0108)
  DISP_SRC = ks0108.c font-ks0066-ru-08.c font-ks0066-ru-24.c font-digits-32.c
else ifeq ($(DISPLAY), KS0066)
  DISP_SRC = ks0066.c
endif

SRCS = main.c eeprom.c display.c i2c.c ds1307.c audio.c $(SPECT_SRC) $(CTRL_SRC) $(DISP_SRC) $(FM_SRC)

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

OBJS = $(SRCS:.c=.o)

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf $(OBJS) -lm
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $@.elf $@.hex
	$(OBJCOPY) -O binary -R .eeprom -R .nwram  $@.elf $@.bin
	wc -c $@.bin
	rm -f $@.bin

%.o: %.c
	$(CC) $(CFLAGS) -D$(AUDIOPROC) -D$(DISPLAY) -c -o $@ $<

clean:
	rm -f $(TARG).{elf,bin} $(OBJS)

flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U flash:w:$(TARG).hex:i

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0xff:m -U hfuse:w:0xd1:m

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U eeprom:w:eeprom_$(shell echo $(AUDIOPROC) | tr A-Z a-z)_en.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U eeprom:w:eeprom_$(shell echo $(AUDIOPROC) | tr A-Z a-z)_ru.bin:r
