#include "lm7001.h"
#include <util/delay.h>

void lm7001Init(void)
{
	DDR(LM7001_DATA) |= LM7001_DATA_PIN;
	DDR(LM7001_CL) |= LM7001_CL_PIN;
	DDR(LM7001_CE) |= LM7001_CE_PIN;

	PORT(LM7001_DATA) &= ~LM7001_DATA_PIN;
	PORT(LM7001_CL) &= ~LM7001_CL_PIN;
	PORT(LM7001_CE) &= ~LM7001_CE_PIN;

	return;
}

static void lm7001Strob(void)
{
	_delay_us(1.5);
	PORT(LM7001_CL) |= LM7001_CL_PIN;
	_delay_us(1.5);
	PORT(LM7001_CL) &= ~LM7001_CL_PIN;

	return;
}

static void lm7001SendByte(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (data & (1<<i))					/* Set data port to bit value */
			PORT(LM7001_DATA) |= LM7001_DATA_PIN;
		else
			PORT(LM7001_DATA) &= ~LM7001_DATA_PIN;
		lm7001Strob();						/* Strob data bit with CL */
	}

	return;
}

void lm7001SetFreq(uint16_t freq)
{
	/* TODO: calculate lm7001 freq params */
	uint16_t div;

	div = (freq + LM7001_IF) / LM7001_RF;

	PORT(LM7001_CE) |= LM7001_CE_PIN;		/* Start transmit */

	lm7001SendByte(div & 0x00FF);
	lm7001SendByte((div & 0x3F00) >> 8);
	lm7001SendByte(LM7001_CTRL_WORD);

	PORT(LM7001_CE) &= ~LM7001_CE_PIN;		/* Finish transmit */

	return;
}
