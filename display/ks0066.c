#include "ks0066.h"

#include <util/delay.h>

#include "../pins.h"
#if defined(KS0066_WIRE_PCF8574)
#include "../i2c.h"
#include <avr/pgmspace.h>
#endif

#define swap(x) (__builtin_avr_swap(x))             // Swaps nibbles

#if defined(KS0066_WIRE_PCF8574)
static uint8_t i2cData;
static uint8_t pcf8574Addr = 0x40;
#endif

static void ks0066WriteStrob()
{
#if defined(KS0066_WIRE_PCF8574)
    I2CWriteByte(i2cData | PCF8574_E_LINE);
    I2CWriteByte(i2cData);
#else
    _delay_us(0.04);
    SET(KS0066_E);
    _delay_us(0.23);
    CLR(KS0066_E);
#endif
}

static void ks0066SetData(uint8_t data)
{
#if defined(KS0066_WIRE_PCF8574)
    i2cData &= 0x0F;
    i2cData |= (data & 0xF0);
#else
    if (data & (1 << 7)) SET(KS0066_D7);
    else CLR(KS0066_D7);
    if (data & (1 << 6)) SET(KS0066_D6);
    else CLR(KS0066_D6);
    if (data & (1 << 5)) SET(KS0066_D5);
    else CLR(KS0066_D5);
    if (data & (1 << 4)) SET(KS0066_D4);
    else CLR(KS0066_D4);
#if defined(KS0066_WIRE_8BIT)
    if (data & (1 << 3)) SET(KS0066_D3);
    else CLR(KS0066_D3);
    if (data & (1 << 2)) SET(KS0066_D2);
    else CLR(KS0066_D2);
    if (data & (1 << 1)) SET(KS0066_D1);
    else CLR(KS0066_D1);
    if (data & (1 << 0)) SET(KS0066_D0);
    else CLR(KS0066_D0);
#endif
#endif
}

static void ks0066WritePort(uint8_t data)
{
#if defined(KS0066_WIRE_PCF8574)
    I2CStart(pcf8574Addr);
    i2cData &= ~PCF8574_RW_LINE;
#else
    _delay_us(100);
#endif

    ks0066SetData(data);
    ks0066WriteStrob();

#if !defined(KS0066_WIRE_8BIT)
    ks0066SetData(swap(data));
    ks0066WriteStrob();
#endif

#if defined(KS0066_WIRE_PCF8574)
    I2CStop();
#endif
}

void ks0066WriteCommand(uint8_t command)
{
#if defined(KS0066_WIRE_PCF8574)
    i2cData &= ~PCF8574_RS_LINE;
#else
    CLR(KS0066_RS);
#endif
    ks0066WritePort(command);
}

void ks0066WriteData(uint8_t data)
{
#if defined(KS0066_WIRE_PCF8574)
    i2cData |= PCF8574_RS_LINE;
#else
    SET(KS0066_RS);
#endif
    ks0066WritePort(data);
}

void ks0066Clear()
{
    ks0066WriteCommand(KS0066_CLEAR);
    _delay_ms(2);
}

void ks0066Init()
{
#if defined(KS0066_WIRE_PCF8574)
    I2CStart(pcf8574Addr);

    i2cData = PCF8574_BL_LINE;
#else
    OUT(KS0066_BCKL);
    OUT(KS0066_E);
    OUT(KS0066_RW);
    OUT(KS0066_RS);

    OUT(KS0066_D7);
    OUT(KS0066_D6);
    OUT(KS0066_D5);
    OUT(KS0066_D4);
#if defined(KS0066_WIRE_8BIT)
    OUT(KS0066_D3);
    OUT(KS0066_D2);
    OUT(KS0066_D1);
    OUT(KS0066_D0);
#endif

    SET(KS0066_BCKL);
    CLR(KS0066_E);
    CLR(KS0066_RW);
    CLR(KS0066_RS);
#endif

    ks0066SetData(KS0066_FUNCTION | KS0066_8BIT);   // Init data
    ks0066WriteStrob();
    _delay_ms(20);
    ks0066WriteStrob();
    _delay_ms(5);
    ks0066WriteStrob();
    _delay_us(120);
    ks0066WriteStrob();

#if defined(KS0066_WIRE_8BIT)
    ks0066WriteCommand(KS0066_FUNCTION | KS0066_8BIT | KS0066_2LINES);
#else
#if defined(KS0066_WIRE_PCF8574)
    i2cData &= ~PCF8574_RW_LINE;
    i2cData &= ~PCF8574_RS_LINE;
#else
    CLR(KS0066_RS);
    _delay_us(100);
#endif
    ks0066SetData(KS0066_FUNCTION);
    ks0066WriteStrob();
#if defined(KS0066_WIRE_PCF8574)
    I2CStop();
#endif
    ks0066WriteCommand(KS0066_FUNCTION | KS0066_2LINES);
#endif
    ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
    ks0066WriteCommand(KS0066_CLEAR);
    _delay_ms(2);
    ks0066WriteCommand(KS0066_SET_MODE | KS0066_INC_ADDR);
}

void ks0066SelectSymbol(uint8_t num)
{
    ks0066WriteCommand(KS0066_SET_CGRAM + num * 8);
}

void ks0066SetXY(uint8_t x, uint8_t y)
{
    ks0066WriteCommand(KS0066_SET_DDRAM + (y ? KS0066_LINE_WIDTH : 0) + x);
}

void ks0066WriteString(char *string)
{
    while (*string)
        ks0066WriteData(*string++);
}

void pcf8574SetBacklight(uint8_t value)
{
#if defined(KS0066_WIRE_PCF8574)
    if (value)
        i2cData |= PCF8574_BL_LINE;
    else
        i2cData &= ~PCF8574_BL_LINE;
    ks0066WriteCommand(KS0066_NO_COMMAND);
#endif
}
