#!/bin/sh

for DISPLAY in KS0108A KS0108B ST7920
do
	make clean
	make DISPLAY=${DISPLAY}
done

make clean
