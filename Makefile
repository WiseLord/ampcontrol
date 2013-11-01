TARG=ampcontrol

SRCS = main.c ks0108.c fft.c adc.c input.c
MCU = atmega32
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
	rm -f *.elf *.bin *.hex $(OBJS) *.map

flash: $(TARG)
	$(AVRDUDE) -p m32 -B 1.1 -U flash:w:$(TARG).hex:i

fuse:
	$(AVRDUDE) -p m32 -U lfuse:w:0xff:m -U hfuse:w:0xd9:m
