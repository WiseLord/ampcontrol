#include "tea5767.h"

#include "../i2c.h"

static uint8_t wrBuf[5];
static uint8_t rdBuf[5];

static uint8_t ctrl = 0x79;

static void tea5767WriteI2C(void)
{
    uint8_t i;

    I2CStart(TEA5767_I2C_ADDR);
    for (i = 0; i < sizeof(wrBuf); i++)
        I2CWriteByte(wrBuf[i]);
    I2CStop();

    return;
}

void tea5767Init(uint8_t tea5767Ctrl)
{
    uint8_t tmp;

    ctrl = tea5767Ctrl;

    wrBuf[0] = TEA5767_MUTE;

    wrBuf[1] = 0;

    wrBuf[2] = TEA5767_HLSI;

    if (ctrl & TEA5767_CTRL_HCC)
        tmp = TEA5767_HCC;
    if (ctrl & TEA5767_CTRL_SNC)
        tmp |= TEA5767_SNC;
    if (ctrl & TEA5767_CTRL_SMUTE)
        tmp |= TEA5767_SMUTE;
    if (ctrl & TEA5767_CTRL_BL)
        tmp |= TEA5767_BL;
    if (ctrl & TEA5767_CTRL_XTAL)
        tmp |= TEA5767_XTAL;
    wrBuf[3] = tmp;

    if (ctrl & TEA5767_CTRL_DTC)
        tmp = TEA5767_DTC;
    if (ctrl & TEA5767_CTRL_PLLREF)
        tmp |= TEA5767_PLLREF;
    wrBuf[4] = tmp;

    return;
}

void tea5767SetFreq(uint16_t freq, uint8_t mono)
{
    uint16_t div;

    if (ctrl & TEA5767_CTRL_XTAL)
        div = ((uint32_t)freq * 10000 + 225000) / 8192;
    else
        div = (freq * 4 + 90) / 5;

    wrBuf[0] &= 0xC0;
    wrBuf[0] |= (div >> 8) & 0x3F;

    wrBuf[1] = div & 0xFF;

    if (mono)
        wrBuf[2] |= TEA5767_MS;
    else
        wrBuf[2] &= ~TEA5767_MS;

    tea5767WriteI2C();

    return;
}

uint8_t *tea5767ReadStatus(void)
{
    uint8_t i;

    I2CStart(TEA5767_I2C_ADDR | I2C_READ);
    for (i = 0; i < sizeof(rdBuf) - 1; i++)
        I2CReadByte(&rdBuf[i], I2C_ACK);
    I2CReadByte(&rdBuf[sizeof(rdBuf) - 1], I2C_NOACK);
    I2CStop();

    return rdBuf;
}

void tea5767SetMute(uint8_t mute)
{
    if (mute)
        wrBuf[0] |= TEA5767_MUTE;
    else
        wrBuf[0] &= ~TEA5767_MUTE;

    tea5767WriteI2C();

    return;
}

void tea5767PowerOn(void)
{
    wrBuf[0] &= ~TEA5767_MUTE;
    wrBuf[3] &= ~TEA5767_STBY;

    return;
}

void tea5767PowerOff(void)
{
    wrBuf[3] |= TEA5767_STBY;

    tea5767SetMute(1);

    return;
}
