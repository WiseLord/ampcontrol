TARG=ampcontrol

SRCS = main.c ks0108.c font-ks0066-ru-08.c font-ks0066-ru-24.c font-digits-32.c fft.c adc.c input.c i2c.c param.c ds1307.c
MCU = atmega16
F_CPU = 16000000L

CS = -fexec-charset=ks0066-ru

OPTIMIZE = -Os -mcall-prologues
CFLAGS = -g -Wall -Werror -lm $(OPTIMIZE) $(CS) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS = -g -Wall -Werror -mmcu=$(MCU)
OBJS = $(SRCS:.c=.o)

CC = avr-gcc
OBJCOPY = avr-objcopy

AVRDUDE = avrdude
AD_MCU = m16
#AD_PROG = stk500v2
#AD_PORT = avrdoper
AD_PROG = usbasp
AD_PORT = usbasp

AD_CMDLINE = -p $(AD_MCU) -c $(AD_PROG) -P $(AD_PORT)

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf  $(OBJS) -lm
#	$(OBJCOPY) -O binary -R .eeprom -R .nwram  $@.elf $@.bin
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $@.elf $@.hex

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARG).elf $(TARG).bin $(TARG).hex $(OBJS) *.map

flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U flash:w:$(TARG).hex:i

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0xff:m -U hfuse:w:0xd1:m

eeprom_tda7439:
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U eeprom:w:eeprom_tda7439.bin:r

eeprom_tda7313:
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U eeprom:w:eeprom_tda7313.bin:r

eeprom_tda7318:
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U eeprom:w:eeprom_tda7318.bin:r
