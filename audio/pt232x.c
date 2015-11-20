#include "pt232x.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

static sndParam *sndPar;

void pt232xInit(sndParam *sp)
{
	sndPar = sp;

	return;
}
