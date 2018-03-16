#include "tux032.h"
#include "tuner.h"

#include "../i2c.h"

static uint8_t wrBuf[9] = {0x80, 0x00, 0x00, 0x64, 0xB1, 0xC6, 0x4B, 0xA2, 0xD2};

static void tux032WriteI2C(uint8_t bytes)
{
    uint8_t i;

    I2CStart(TUX032_I2C_ADDR);
    for (i = 0; i < bytes; i++)
        I2CWriteByte(wrBuf[i]);
    I2CStop();
}

void tux032Init()
{
    tux032PowerOff();
}

void tux032SetFreq()
{
    uint16_t freq = tuner.freq / 5 + 214;

    wrBuf[0] = 0x80;
    wrBuf[1] = freq >> 8;
    wrBuf[2] = freq & 0xFF;

    tux032WriteI2C(sizeof(wrBuf));
}

void tux032ReadStatus()
{
    uint8_t i;

    I2CStart(TUX032_I2C_ADDR | I2C_READ);
    for (i = 0; i < TUX032_RDBUF_SIZE - 1; i++)
        tunerRdbuf[i] = I2CReadByte(I2C_ACK);
    tunerRdbuf[TUX032_RDBUF_SIZE - 1] = I2CReadByte(I2C_NOACK);
    I2CStop();
}

void tux032SetMute()
{
    if (tuner.mute)
        tux032PowerOff();
    else
        tux032PowerOn();
}

void tux032PowerOn()
{
    wrBuf[0] = 0x82;
    wrBuf[1] = 0x64;

    tux032WriteI2C(2);
}

void tux032PowerOff()
{
    wrBuf[0] = 0x82;
    wrBuf[1] = 0x00;

    tux032WriteI2C(2);
}
