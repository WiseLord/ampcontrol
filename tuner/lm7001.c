#include "lm7001.h"

#include "tuner.h"
#include "../spisw.h"
#include "../pins.h"

void lm7001Init(void)
{
	SPIswInit(SPISW_DORD_LSB_FIRST);

	PORT(SPISW_CE) &= ~SPISW_CE_LINE;

	return;
}

void lm7001SetFreq(void)
{
	uint16_t div;

	div = (tuner.freq + LM7001_IF) / LM7001_RF;

	PORT(SPISW_CE) |= SPISW_CE_LINE;

	SPIswSendByte(div & 0x00FF);
	SPIswSendByte((div & 0x3F00) >> 8);
	SPIswSendByte(LM7001_CTRL_WORD);

	PORT(SPISW_CE) &= ~SPISW_CE_LINE;

	return;
}
