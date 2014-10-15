#!/bin/sh

make clean

for AUDIOPROC in TDA7313 TDA7318 TDA7439
do
	make clean
	make AUDIOPROC=${AUDIOPROC}
done

make clean
