#!/bin/sh

for WIRE in 4BIT PCF8574
do
	for AUDIOPROC in PGA2310 PT232X RDA580X_AUDIO TDA731X TDA7439 TDA7448 TEA63X0
	do
		for TUNER in LC72131 LM7001 RDA580X TEA5767 TUX032
		do
			if [ $AUDIOPROC != "PGA2310" ] || [ $TUNER != "LC72131" -a $TUNER != "LM7001" ]
			then
				if [ $AUDIOPROC != "RDA580X_AUDIO" -o $TUNER  == "RDA580X" ]
				then
					make clean
					make APROC_LIST=${AUDIOPROC} TUNER_LIST=${TUNER} WIRE=${WIRE}
				fi
			fi
		done
	done
done

make clean
