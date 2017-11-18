#include "spisw.h"

#include <util/delay.h>
#include "pins.h"

static uint8_t dord = SPISW_DORD_MSB_FIRST;
static uint8_t extFunc;

static void SPIswStrob()
{
    _delay_us(1.5);
    PORT(SPISW_CLK) |= SPISW_CLK_LINE;
    _delay_us(1.5);
    PORT(SPISW_CLK) &= ~SPISW_CLK_LINE;
}

void SPIswInitLines(uint8_t func)
{
    DDR(SPISW_DI) |= SPISW_DI_LINE;
    DDR(SPISW_CLK) |= SPISW_CLK_LINE;
    DDR(SPISW_CE) |= SPISW_CE_LINE;

    extFunc = func;
}

void SPIswInit(uint8_t dataOrder)
{
    dord = dataOrder;

    PORT(SPISW_CLK) &= ~SPISW_CLK_LINE;
}

void SPIswSendByte(uint8_t data)
{
    uint8_t i;
    uint8_t mask = SPISW_DORD_LSB_FIRST ? 0x01 : 0x80;

    for (i = 0; i < 8; i++) {
        if (data & mask)
            PORT(SPISW_DI) |= SPISW_DI_LINE;
        else
            PORT(SPISW_DI) &= ~SPISW_DI_LINE;

        SPIswStrob();

        if (dord == SPISW_DORD_LSB_FIRST)
            data >>= 1;
        else
            data <<= 1;
    }
}

void SPIswSet(int8_t input)
{
    if (extFunc != USE_INPUT_STATUS)
        return;

    PORT(EXT_0) &= ~EXT_0_LINE;
    PORT(EXT_1) &= ~EXT_1_LINE;
    PORT(EXT_2) &= ~EXT_2_LINE;

    switch (input) {
    case 0:
        PORT(EXT_0) |= EXT_0_LINE;
        break;
    case 1:
        PORT(EXT_1) |= EXT_1_LINE;
        break;
    case 2:
        PORT(EXT_2) |= EXT_2_LINE;
        break;
    default:
        break;
    }
}
