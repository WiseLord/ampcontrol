#!/bin/sh

cd src/

MAKEOPTS="-j4 -s"

for MCU in atmega16 atmega32
do
  for DISPLAY in KS0066_16X2_8BIT KS0066_16X2_PCF8574 KS0108
  do
    for AUDIOPROC in PGA2310 PT232X R2S15904SP TDA731X TDA7439 TDA7448 TEA63X0
    do
      for TUNER in LC72131 LM7001 RDA580X SI470X TEA5767 TUX032
      do
        if [ $AUDIOPROC != "PGA2310" -a $AUDIOPROC != "R2S15904SP" ] || [ $TUNER != "LC72131" -a $TUNER != "LM7001" ]
        then
          if [ $AUDIOPROC != "TUNER_AUDIO" -o $TUNER == "RDA580X" -o $TUNER == "SI470X" ]
          then
            make clean
            echo "Compiling for MCU=${MCU} APROC=${AUDIOPROC} TUNER=${TUNER} DISPLAY=${DISPLAY}"
            make ${MAKEOPTS} -j4  MCU=${MCU} APROC_LIST=${AUDIOPROC} TUNER_LIST=${TUNER} DISPLAY=${DISPLAY}
          fi
        fi
      done
    done
  done
done

make clean
