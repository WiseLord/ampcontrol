#!/bin/sh

for MCU in atmega168p atmega328p atmega8
do
  for WIRE in 4BIT PCF8574
  do
    for AUDIOPROC in PGA2310 PT232X R2S15904SP TDA731X TDA7439 TDA7448 TEA63X0 TUNER_AUDIO
    do
      for TUNER in LC72131 LM7001 RDA580X SI470X TEA5767 TUX032
      do
        if [ $AUDIOPROC != "PGA2310" -a $AUDIOPROC != "R2S15904SP" ] || [ $TUNER != "LC72131" -a $TUNER != "LM7001" ]
        then
          if [ $AUDIOPROC != "TUNER_AUDIO" -o $TUNER == "RDA580X" -o $TUNER == "SI470X" ]
          then
            make clean
            make -j4  MCU=${MCU} APROC_LIST=${AUDIOPROC} TUNER_LIST=${TUNER} WIRE=${WIRE}
          fi
        fi
      done
    done
  done
done

make clean
