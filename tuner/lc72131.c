#include "lc72131.h"

#include "tuner.h"
#include "../spisw.h"
#include "../pins.h"

void lc72131Init(void)
{
    SPIswInit(SPISW_DORD_LSB_FIRST);

    PORT(SPISW_CE) &= ~SPISW_CE_LINE;

    return;
}

void lc72131SetFreq(void)
{
    uint16_t div;

    div = (tuner.freq + LC72131_IF) / LC72131_RF;

    SPIswSendByte(LC72131_IO_IN1);
    PORT(SPISW_CE) |= SPISW_CE_LINE;
    SPIswSendByte(div & 0x00FF);
    SPIswSendByte((div & 0xFF00) >> 8);
    SPIswSendByte(LC72131_CTRL_IN1);
    PORT(SPISW_CE) &= ~SPISW_CE_LINE;

    SPIswSendByte(LC72131_IO_IN2);
    PORT(SPISW_CE) |= SPISW_CE_LINE;
    SPIswSendByte(LC72131_CTRL_IN2_1);
    SPIswSendByte(LC72131_CTRL_IN2_2);
    SPIswSendByte(LC72131_CTRL_IN2_3);
    PORT(SPISW_CE) &= ~SPISW_CE_LINE;

    return;
}
