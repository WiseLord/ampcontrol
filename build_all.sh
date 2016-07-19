#!/bin/sh

for WIRE in 4BIT PCF8574
do
	for AUDIOPROC in PGA2310 TDA731X TDA7439 TDA7448 TEA6330
	do
		for TUNER in LM7001 RDA580X TEA5767 TUX032
		do
			if [ $AUDIOPROC != "PGA2310" -o $TUNER != "LM7001" ]
			then
				make clean
				make AUDIOPROC=${AUDIOPROC} TUNER=${TUNER} WIRE=${WIRE}
			fi
		done
	done
done

make clean
