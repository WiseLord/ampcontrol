#include "tda7439.h"

#include <avr/pgmspace.h>

static const sndGrid tda7439Grid[] PROGMEM = {
	{-79,  0, 1 * 8},	/* 0: Volume */
	{ -7,  7, 2 * 8},	/* 1: Bass/middle/treble */
	{-47,  0, 1 * 8},	/* 2: Preamp */
	{-21, 21, 1 * 8},	/* 3: Balance */
	{  0, 15, 2 * 8},	/* 4: Gain */
};

const sndGrid *tda7439SndGrid(sndMode index)
{
	const sndGrid *ret;

	if (index == MODE_SND_VOLUME)
		ret = &tda7439Grid[0];
	else if (index == MODE_SND_BASS || index == MODE_SND_MIDDLE ||
	         index == MODE_SND_TREBLE)
		ret = &tda7439Grid[1];
	else if (index == MODE_SND_PREAMP)
		ret = &tda7439Grid[2];
	else if (index == MODE_SND_BALANCE)
		ret = &tda7439Grid[3];
	else if (index == MODE_SND_GAIN0 || index == MODE_SND_GAIN1 ||
	         index == MODE_SND_GAIN2 || index == MODE_SND_GAIN3)
		ret = &tda7439Grid[4];
	else
		ret = (sndGrid*)0;

	return ret;
}
