#!/bin/sh

make AUDIOPROC=TDA7439 TARG=ampcontrol_tda7439_gd
make AUDIOPROC=TDA7439 TARG=ampcontrol_tda7439_gd clean
make AUDIOPROC=TDA7313 TARG=ampcontrol_tda7313_gd
make AUDIOPROC=TDA7439 TARG=ampcontrol_tda7439_gd clean
make AUDIOPROC=TDA7318 TARG=ampcontrol_tda7318_gd
make AUDIOPROC=TDA7439 TARG=ampcontrol_tda7439_gd clean
