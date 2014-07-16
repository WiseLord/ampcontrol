#include "lm7001.h"
#include <util/delay.h>

void lm7001Init(void)
{
	LM7001_DDR |= LM7001_DATA | LM7001_CL | LM7001_CE;
	LM7001_PORT &= ~(LM7001_DATA | LM7001_CL | LM7001_CE);

	return;
}

static void lm7001Strob(void)
{
	_delay_us(1.5);
	LM7001_PORT |= LM7001_CL;
	_delay_us(1.5);
	LM7001_PORT &= ~LM7001_CL;

	return;
}

static void lm7001SendByte(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (data & (1<<i))					/* Set data port to bit value */
			LM7001_PORT |= LM7001_DATA;
		else
			LM7001_PORT &= ~LM7001_DATA;
		lm7001Strob();						/* Strob data bit with CL */
	}

	return;
}

void lm7001SetFreq(uint16_t freq)
{
	/* TODO: calculate lm7001 freq params */
	uint16_t div;

	div = (freq + LM7001_IF) / LM7001_RF;

	LM7001_PORT |= LM7001_CE;				/* Start transmit */

	lm7001SendByte(div & 0x00FF);
	lm7001SendByte((div & 0x3F00) >> 8);
	lm7001SendByte(LM7001_CTRL_WORD);

	LM7001_PORT &= ~LM7001_CE;				/* Finish transmit */

	return;
}
