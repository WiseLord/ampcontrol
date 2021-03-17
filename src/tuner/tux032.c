#include "tux032.h"
#include "tuner.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

static void tux032WriteI2C(uint8_t bytes)
{
    uint8_t i;

    I2CStart(TUX032_I2C_ADDR);
    for (i = 0; i < bytes; i++)
        I2CWriteByte(tunerWrBuf[i]);
    I2CStop();
}

void tux032Init()
{
    tunerWrBuf[3] = 0x64;
    tunerWrBuf[4] = 0xB1;
    tunerWrBuf[5] = 0xC6;
    tunerWrBuf[6] = 0x4B;
    tunerWrBuf[7] = 0xA2;
    tunerWrBuf[8] = 0xD2;

    tux032PowerOff();
}

void tux032SetFreq()
{
    uint16_t freq = tuner.freq / 5 + 214;

    tunerWrBuf[0] = 0x80;
    tunerWrBuf[1] = freq >> 8;
    tunerWrBuf[2] = freq & 0xFF;

    tux032WriteI2C(TUX032_WRBUF_SIZE);
}

void tux032ReadStatus()
{
    uint8_t i;

    I2CStart(TUX032_I2C_ADDR | I2C_READ);
    for (i = 0; i < TUX032_RDBUF_SIZE - 1; i++)
        tunerRdBuf[i] = I2CReadByte(I2C_ACK);
    tunerRdBuf[TUX032_RDBUF_SIZE - 1] = I2CReadByte(I2C_NOACK);
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
    tunerWrBuf[0] = 0x82;
    tunerWrBuf[1] = 0x64;

    tux032WriteI2C(2);
}

void tux032PowerOff()
{
    tunerWrBuf[0] = 0x82;
    tunerWrBuf[1] = 0x00;

    tux032WriteI2C(2);
}
