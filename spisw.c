#include "spisw.h"

#include <util/delay.h>
#include "pins.h"

static uint8_t dord = SPISW_DORD_MSB_FIRST;
static uint8_t extFunc;

static void SPIswStrob()
{
    _delay_us(1.5);
    SET(SPISW_CLK);
    _delay_us(1.5);
    CLR(SPISW_CLK);
}

void SPIswInitLines(uint8_t func)
{
    OUT(SPISW_DI);
    OUT(SPISW_CLK);
    OUT(SPISW_CE);

    extFunc = func;
}

void SPIswInit(uint8_t dataOrder)
{
    dord = dataOrder;

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
}

void SPIswSet(int8_t input)
{
#ifdef _atmega32
    if (extFunc != USE_INPUT_STATUS)
        return;

    CLR(EXT_0);
    CLR(EXT_1);
    CLR(EXT_2);

    switch (input) {
    case 0:
        SET(EXT_0);
        break;
    case 1:
        SET(EXT_1);
        break;
    case 2:
        SET(EXT_2);
        break;
    default:
        break;
    }
#endif
}
