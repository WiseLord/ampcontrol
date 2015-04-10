#include "pga2310.h"

#include <avr/pgmspace.h>

static sndParam *sndPar;

void pga2310Init(sndParam *sp)
{
	sndPar = sp;

	return;
}

void pga2310SetSpeakers(int8_t val)
{
	int8_t spLeft = sndPar[MODE_SND_VOLUME].value;
	int8_t spRight = sndPar[MODE_SND_VOLUME].value;
	int8_t volMin = pgm_read_byte(&sndPar[MODE_SND_VOLUME].grid->min);

	if (sndPar[MODE_SND_BALANCE].value > 0) {
		spLeft -= sndPar[MODE_SND_BALANCE].value;
		if (spLeft < volMin)
			spLeft = volMin;
	} else {
		spRight += sndPar[MODE_SND_BALANCE].value;
		if (spRight < volMin)
			spRight = volMin	;
	}

	/* Write some bytes to bus to set speakers */

	return;
}

void pga2310SetMute(uint8_t val)
{
	if (val == MUTE_ON) {
		/* Write some bytes to bus to set mute */

	} else {
		pga2310SetSpeakers(0);
	}

	return;
}
