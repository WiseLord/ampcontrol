#include "spisw.h"

#include <util/delay.h>
#include "pins.h"

static uint8_t dord = SPISW_DORD_MSB_FIRST;

static void SPIswStrob(void)
{
    _delay_us(1.5);
    SET(SPISW_CLK);
    _delay_us(1.5);
    CLR(SPISW_CLK);

    return;
}

void SPIswInit(uint8_t dataOrder)
{
    dord = dataOrder;

    OUT(SPISW_DI);
    OUT(SPISW_CLK);
    OUT(SPISW_CE);

    CLR(SPISW_CLK);
}

void SPIswSendByte(uint8_t data)
{
    uint8_t i;
    uint8_t mask = SPISW_DORD_LSB_FIRST ? 0x01 : 0x80;

    for (i = 0; i < 8; i++) {
        if (data & mask)
            SET(SPISW_DI);
        else
            CLR(SPISW_DI);

        SPIswStrob();

        if (dord == SPISW_DORD_LSB_FIRST)
            data >>= 1;
        else
            data <<= 1;
    }

    return;
}
