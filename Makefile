TARG=ampcontrol

SRCS = main.c ks0108.c fft.c

MCU=atmega32

F_CPU=16000000L

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

OBJS = $(SRCS:.c=.o)

CFLAGS = -mmcu=$(MCU) -Wall -g -Os -Werror -lm -mcall-prologues -DF_CPU=$(F_CPU)
LDFLAGS = -mmcu=$(MCU) -Wall -g -Os -Werror

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
	$(AVRDUDE) -p m32 -U flash:w:$(TARG).hex:i

fuse:
	$(AVRDUDE) -p m32 -U lfuse:w:0xff:m -U hfuse:w:0xd9:m
