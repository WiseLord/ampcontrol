#include "tea5767.h"
#include "tuner.h"

#include "../i2c.h"

static uint8_t wrBuf[5] = {
    0,
    0,
    TEA5767_HLSI,
    TEA5767_HCC | TEA5767_SNC | TEA5767_SMUTE | TEA5767_XTAL,
    TEA5767_DTC,
};

static void tea5767WriteI2C()
{
    uint8_t i;

    I2CStart(TEA5767_I2C_ADDR);
    for (i = 0; i < sizeof(wrBuf); i++)
        I2CWriteByte(wrBuf[i]);
    I2CStop();
}

void tea5767Init()
{
    wrBuf[3] = tuner.ctrl & (TEA5767_HCC | TEA5767_SNC | TEA5767_SMUTE | TEA5767_BL | TEA5767_XTAL);
    wrBuf[4] = tuner.ctrl & (TEA5767_DTC | TEA5767_PLLREF);
}

void tea5767SetFreq()
{
    uint16_t pll = tuner.freq * 4 + 90;

    if (tuner.ctrl & TEA5767_XTAL) {
        pll = pll * 10000UL / 32768;
    } else {
        pll = pll / 5;
    }

    wrBuf[0] &= 0xC0;
    wrBuf[0] |= (pll >> 8) & 0x3F;

    wrBuf[1] = pll & 0xFF;

    if (tuner.mono)
        wrBuf[2] |= TEA5767_MS;
    else
        wrBuf[2] &= ~TEA5767_MS;

    tea5767WriteI2C();
}

void tea5767ReadStatus()
{
    uint8_t i;

    I2CStart(TEA5767_I2C_ADDR | I2C_READ);
    for (i = 0; i < TEA5767_RDBUF_SIZE - 1; i++)
        tunerRdbuf[i] = I2CReadByte(I2C_ACK);
    tunerRdbuf[TEA5767_RDBUF_SIZE - 1] = I2CReadByte(I2C_NOACK);
    I2CStop();
}

void tea5767SetMute()
{
    if (tuner.mute)
        wrBuf[0] |= TEA5767_MUTE;
    else
        wrBuf[0] &= ~TEA5767_MUTE;

    tea5767WriteI2C();
}

void tea5767PowerOn()
{
    wrBuf[3] &= ~TEA5767_STBY;
}

void tea5767PowerOff()
{
    wrBuf[3] |= TEA5767_STBY;

    tea5767WriteI2C();
}
