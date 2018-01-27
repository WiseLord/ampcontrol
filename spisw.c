#include "spisw.h"

#include <util/delay.h>
#include "pins.h"

static uint8_t dord = SPISW_DORD_MSB_FIRST;

static void SPIswStrob(void)
{
    _delay_us(1.5);
    PORT(SPISW_CLK) |= SPISW_CLK_LINE;
    _delay_us(1.5);
    PORT(SPISW_CLK) &= ~SPISW_CLK_LINE;

    return;
}

void SPIswInit(uint8_t dataOrder)
{
    dord = dataOrder;

    DDR(SPISW_DI) |= SPISW_DI_LINE;
    DDR(SPISW_CLK) |= SPISW_CLK_LINE;
    DDR(SPISW_CE) |= SPISW_CE_LINE;

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

    return;
}
