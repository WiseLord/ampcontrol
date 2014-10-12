#!/bin/sh

make clean

for AUDIOPROC in TDA7313 TDA7318 TDA7439
do
	for TUNER in LM7001 TEA5767 TUX032
	do
		make clean
		make AUDIOPROC=${AUDIOPROC} TUNER=${TUNER}
	done
done

make clean
