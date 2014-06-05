#!/bin/sh

make clean

for AUDIOPROC in TDA7313 TDA7318 TDA7439
do
	for DISPLAY in KS0066 KS0108 LS020 PCF8574
	do
		for TUNER in TEA5767 TUX032
		do
			make AUDIOPROC=${AUDIOPROC} DISPLAY=${DISPLAY} TUNER=${TUNER}
			make AUDIOPROC=${AUDIOPROC} DISPLAY=${DISPLAY} TUNER=${TUNER} clean_var
		done
	done
done

make clean_const
