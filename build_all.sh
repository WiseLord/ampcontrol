#!/bin/sh

for DISPLAY in KS0108A KS0108B ST7920 SSD1306 KS0066_16X2 KS0066_16X2_PCF8574 LS020
do
	make clean
	make MCU=atmega32 DISPLAY=${DISPLAY}
done

for DISPLAY in KS0108A ST7920 SSD1306
do
	make clean
	make MCU=atmega328p DISPLAY=${DISPLAY}
done

make clean
