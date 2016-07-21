#!/bin/sh

for WIRE in 4BIT PCF8574
do
	for AUDIOPROC in PGA2310 PT232X RDA580X_AUDIO TDA731X TDA7439 TDA7448 TEA6330
	do
		for TUNER in LM7001 RDA580X TEA5767 TUX032
		do
			if [ $AUDIOPROC != "PGA2310" -o $TUNER != "LM7001" ]
			then
				if [ $AUDIOPROC != "RDA580X_AUDIO" -o $TUNER  == "RDA580X" ]
				then
					make clean
					make AUDIOPROC=${AUDIOPROC} TUNER=${TUNER} WIRE=${WIRE}
				fi
			fi
		done
	done
done

make clean
