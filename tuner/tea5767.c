#include "tea5767.h"
#include "tuner.h"

#include "../i2c.h"

static void tea5767WriteI2C()
{
    uint8_t i;

    I2CStart(TEA5767_I2C_ADDR);
    for (i = 0; i < TEA5767_WRBUF_SIZE; i++)
        I2CWriteByte(tunerWrBuf[i]);
    I2CStop();
}

void tea5767Init()
{
    tunerWrBuf[2] = TEA5767_HLSI;
    tunerWrBuf[3] = tuner.ctrl & (TEA5767_HCC | TEA5767_SNC | TEA5767_SMUTE | TEA5767_BL | TEA5767_XTAL);
    tunerWrBuf[4] = tuner.ctrl & (TEA5767_DTC | TEA5767_PLLREF);
}

void tea5767SetFreq()
{
    uint16_t pll = tuner.freq * 4 + 90;

    if (tuner.ctrl & TEA5767_XTAL) {
        pll = pll * 10000UL / 32768;
    } else {
        pll = pll / 5;
    }

    tunerWrBuf[0] &= 0xC0;
    tunerWrBuf[0] |= (pll >> 8) & 0x3F;

    tunerWrBuf[1] = pll & 0xFF;

    if (tuner.mono)
        tunerWrBuf[2] |= TEA5767_MS;
    else
        tunerWrBuf[2] &= ~TEA5767_MS;

    tea5767WriteI2C();
}

void tea5767ReadStatus()
{
    uint8_t i;

    I2CStart(TEA5767_I2C_ADDR | I2C_READ);
    for (i = 0; i < TEA5767_RDBUF_SIZE - 1; i++)
        tunerRdBuf[i] = I2CReadByte(I2C_ACK);
    tunerRdBuf[TEA5767_RDBUF_SIZE - 1] = I2CReadByte(I2C_NOACK);
    I2CStop();
}

void tea5767SetMute()
{
    if (tuner.mute)
        tunerWrBuf[0] |= TEA5767_MUTE;
    else
        tunerWrBuf[0] &= ~TEA5767_MUTE;

    tea5767WriteI2C();
}

void tea5767PowerOn()
{
    tunerWrBuf[3] &= ~TEA5767_STBY;

    tea5767WriteI2C();
}

void tea5767PowerOff()
{
    tunerWrBuf[3] |= TEA5767_STBY;

    tea5767WriteI2C();
}
