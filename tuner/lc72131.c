#include "lc72131.h"
#include "tuner.h"

#include <util/delay.h>
#include "../pins.h"

void lc72131Init(void)
{
	DDR(LC72131_DI) |= LC72131_DI_LINE;
	DDR(LC72131_CL) |= LC72131_CL_LINE;
	DDR(LC72131_CE) |= LC72131_CE_LINE;

	PORT(LC72131_DI) &= ~LC72131_DI_LINE;
	PORT(LC72131_CL) &= ~LC72131_CL_LINE;
	PORT(LC72131_CE) &= ~LC72131_CE_LINE;

	return;
}

static void lc72131Strob(void)
{
	_delay_us(1.5);
	PORT(LC72131_CL) |= LC72131_CL_LINE;
	_delay_us(1.5);
	PORT(LC72131_CL) &= ~LC72131_CL_LINE;

	return;
}

static void lc72131SendByte(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (data & (1<<i))
			PORT(LC72131_DI) |= LC72131_DI_LINE;
		else
			PORT(LC72131_DI) &= ~LC72131_DI_LINE;
		lc72131Strob();
	}

	return;
}

void lc72131SetFreq(void)
{
	uint16_t div;

	div = (tuner.freq + LC72131_IF) / LC72131_RF;

	lc72131SendByte(LC72131_IO_IN1);
	PORT(LC72131_CE) |= LC72131_CE_LINE;
	lc72131SendByte(div & 0x00FF);
	lc72131SendByte((div & 0xFF00) >> 8);
	lc72131SendByte(LC72131_CTRL_IN1);
	PORT(LC72131_CE) &= ~LC72131_CE_LINE;

	lc72131SendByte(LC72131_IO_IN2);
	PORT(LC72131_CE) |= LC72131_CE_LINE;
	lc72131SendByte(LC72131_CTRL_IN2_1);
	lc72131SendByte(LC72131_CTRL_IN2_2);
	lc72131SendByte(LC72131_CTRL_IN2_3);
	PORT(LC72131_CE) &= ~LC72131_CE_LINE;

	return;
}
