#include "pga2310.h"

#include <avr/pgmspace.h>
#include "../pins.h"

static void pga2310Strob(void)
{
	PORT(PGA2310_SCLK) |= PGA2310_SCLK_LINE;
	asm("nop");
	PORT(PGA2310_SCLK) &= ~PGA2310_SCLK_LINE;

	return;
}

static void pga2310SendByte(uint8_t data)
{
	int8_t i;

	for (i = 7; i >= 0; i--) {
		if (data & (1<<i))
			PORT(PGA2310_SDI) |= PGA2310_SDI_LINE;
		else
			PORT(PGA2310_SDI) &= ~PGA2310_SDI_LINE;
		pga2310Strob();
	}

	return;
}

void pga2310SendGainLevels(uint8_t right, uint8_t left)
{
	PORT(PGA2310_CS) &= ~PGA2310_CS_LINE;
	pga2310SendByte(right << 1);
	pga2310SendByte(left << 1);
	PORT(PGA2310_CS) |= PGA2310_CS_LINE;

	return;
}

void pga2310Init(sndParam *sp)
{
	DDR(PGA2310_SDI) |= PGA2310_SDI_LINE;
	DDR(PGA2310_SCLK) |= PGA2310_SCLK_LINE;
	DDR(PGA2310_CS) |= PGA2310_CS_LINE;

	PORT(PGA2310_SDI) |= PGA2310_SDI_LINE;
	PORT(PGA2310_SCLK) &= ~PGA2310_SCLK_LINE;
	PORT(PGA2310_CS) |= PGA2310_CS_LINE;

	return;
}

void pga2310SetSpeakers(void)
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
			spRight = volMin;
	}

	pga2310SendGainLevels(96 + spRight, 96 + spLeft);

	return;
}

void pga2310SetMute(void)
{
	if (aproc.mute) {
		pga2310SendGainLevels(PGA2310_MUTE, PGA2310_MUTE);
	} else {
		pga2310SetSpeakers();
	}

	return;
}
