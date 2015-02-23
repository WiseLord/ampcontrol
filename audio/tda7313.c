#include "tda7313.h"

#include <avr/pgmspace.h>

static const sndGrid tda7313Grid[] PROGMEM = {
	{-63,  0, 1.25 * 8},	/* 0: Volume */
	{ -7,  7, 2.00 * 8},	/* 1: Bass/treble */
	{-15, 15, 1.25 * 8},	/* 2: FrontRear/balance */
	{  0,  3, 3.75 * 8},	/* 3: Gain */
};

const sndGrid *tda7313SndGrid(sndMode index)
{
	const sndGrid *ret;

	if (index == MODE_SND_VOLUME)
		ret = &tda7313Grid[0];
	else if (index == MODE_SND_BASS || index == MODE_SND_TREBLE)
		ret = &tda7313Grid[1];
	else if (index == MODE_SND_FRONTREAR || index == MODE_SND_BALANCE)
		ret = &tda7313Grid[2];
	else if (index == MODE_SND_GAIN0 || index == MODE_SND_GAIN1 ||
	         index == MODE_SND_GAIN2)
		ret = &tda7313Grid[3];
	else
		ret = (sndGrid*)0;

	return ret;
}
