#!/bin/sh

make clean

for WIRE in 4BIT PCF8574
do
	for AUDIOPROC in TDA7313 TDA7318 TDA7439
	do
		for TUNER in LM7001 NOTUNER RDA5807 TEA5767 TUX032
		do
			make clean
			make AUDIOPROC=${AUDIOPROC} TUNER=${TUNER} WIRE=${WIRE}
		done
	done
done

make clean
