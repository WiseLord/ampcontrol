#!/bin/sh

make clean

for WIRE in 4BIT PCF8574
do
	for AUDIOPROC in TDA731X TDA7439 TDA7448 TEA6330
	do
		for TUNER in LM7001 RDA580X TEA5767 TUX032
		do
			make clean
			make AUDIOPROC=${AUDIOPROC} TUNER=${TUNER} WIRE=${WIRE}
		done
	done
done

make clean
