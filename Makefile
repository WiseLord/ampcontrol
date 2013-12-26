TARG=ampcontrol

SRCS = main.c ks0108.c fft.c adc.c input.c i2c.c param.c ds1307.c
MCU = atmega16
F_CPU = 16000000L

CS = -fexec-charset=k1013vg6-0

OPTIMIZE = -Os -mcall-prologues
CFLAGS = -g -Wall -Werror -lm $(OPTIMIZE) $(CS) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS = -g -Wall -Werror -mmcu=$(MCU)
OBJS = $(SRCS:.c=.o)

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf  $(OBJS) -lm
	$(OBJCOPY) -O binary -R .eeprom -R .nwram  $@.elf $@.bin
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $@.elf $@.hex

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARG).elf $(TARG).bin $(TARG).hex $(OBJS) *.map

flash: $(TARG)
	$(AVRDUDE) -Vq -p m16 -B 1.1 -U flash:w:$(TARG).hex:i

fuse:
	$(AVRDUDE) -p m16 -U lfuse:w:0xff:m -U hfuse:w:0xd1:m

eeprom:
	$(AVRDUDE) -p m16 -U eeprom:w:eeprom.bin:r
