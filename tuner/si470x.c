#include "si470x.h"
#include "tuner.h"

#include "../i2c.h"
#include "../pins.h"
#include <util/delay.h>

static uint8_t wrBuf[12];

static void si470xWriteI2C(uint8_t bytes)
{
    uint8_t i;

    I2CStart(SI470X_I2C_ADDR);
    for (i = 0; i < bytes; i++)
        I2CWriteByte(wrBuf[i]);
    I2CStop();
}

static void initRegs()
{
    uint8_t i;

    I2CStart(SI470X_I2C_ADDR | I2C_READ);

    for (i = 0; i < 26; i++) {
        // Skip regs 0A..06
        I2CReadByte(I2C_ACK);
    }

    // Regs 07..09 must be read first before we will write them later
    wrBuf[10] = I2CReadByte(I2C_ACK);
    wrBuf[11] = I2CReadByte(I2C_NOACK);

    I2CStop();
}

void si470xReset()
{
    OUT(SI470X_B_RST);
    OUT(SI470X_B_SDIO);

    CLR(SI470X_B_SDIO);
    CLR(SI470X_B_RST);
    _delay_ms(1);
    SET(SI470X_B_RST);
    _delay_ms(1);
}

void si470xInit()
{
    initRegs();

    wrBuf[0] = /*SI470X_DMUTE | */SI470X_SEEKUP;
    wrBuf[4] = SI470X_DE; // 50us used in Europe
    wrBuf[7] = SI470X_BAND_JAPAN_WIDE | SI470X_SPACE_100;

    wrBuf[6] = SI470X_SEEKTH & 12; // 25 by default for backward compatibility
    wrBuf[9] = (SI470X_SKSNR & 0b00010000) | (SI470X_SKCNT & 0b00000001);

    wrBuf[10] |= SI470X_XOSCEN;

    wrBuf[0] |= SI470X_RDSM; // New method
    wrBuf[4] |= SI470X_RDS;

    si470xWriteI2C(sizeof(wrBuf));
}

void si470xSetFreq()
{
    uint16_t chan;

    chan = (tuner.freq - 7600) / SI470X_CHAN_SPACING;

    wrBuf[0] &= ~SI470X_SEEK; // not seek
    wrBuf[2] = SI470X_TUNE | ((chan >> 8) & 0b00000011);
    wrBuf[3] = (chan & SI470X_CHAN_7_0);

    si470xWriteI2C(4);
}

void si470xReadStatus()
{
    uint8_t i;

    I2CStart(SI470X_I2C_ADDR | I2C_READ);
    for (i = 0; i < SI470X_RDBUF_SIZE - 1; i++)
        tunerRdbuf[i] = I2CReadByte(I2C_ACK);
    tunerRdbuf[SI470X_RDBUF_SIZE - 1] = I2CReadByte(I2C_NOACK);
    I2CStop();

    if (tunerRdbuf[0] & SI740X_STC) { // seek complete
        wrBuf[0] &= ~SI470X_SEEK;
        wrBuf[2] &= ~SI470X_TUNE;
        si470xWriteI2C(4);
    }

    // Get RDS data
#ifdef _RDS
    if (tuner.rds) {
        // If RDS ready and sync flag is set
        if ((tunerRdbuf[0] & SI740X_RDSR) && (tunerRdbuf[0] & SI740X_RDSS)) {
            // If there are no non-correctable errors in blocks A-D
            if (    (tunerRdbuf[0] & SI740X_BLERA) != SI740X_BLERA &&
                    (tunerRdbuf[2] & SI740X_BLERB) != SI740X_BLERB &&
                    (tunerRdbuf[2] & SI740X_BLERC) != SI740X_BLERC &&
                    (tunerRdbuf[2] & SI740X_BLERD) != SI740X_BLERD ) {
                // Send rdBuf[4..11] as 16-bit blocks A-D
                rdsSetBlocks(&tunerRdbuf[4]);
            }
        }
    }
#endif

    uint16_t chan = tunerRdbuf[2] & SI740X_READCHAN_9_8;
    chan <<= 8;
    chan |= tunerRdbuf[3];

    tuner.rdFreq = chan * SI470X_CHAN_SPACING + 7600;
}

void si470xSetVolume(int8_t value)
{
    wrBuf[7] &= ~SI470X_VOLUME;
    wrBuf[7] |= value;

    si470xWriteI2C(8);
}

void si470xSetMute(uint8_t value)
{
    if (value) {
        wrBuf[0] &= ~SI470X_DMUTE;
    } else {
        wrBuf[0] |= SI470X_DMUTE;
    }

    si470xWriteI2C(2);
}

void si470xSetMono(uint8_t value)
{
    if (value) {
        wrBuf[0] |= SI470X_MONO;
    } else {
        wrBuf[0] &= ~SI470X_MONO;
    }

    si470xWriteI2C(2);
}

#ifdef _RDS
void si470xSetRds(uint8_t value)
{
    rdsDisable();

    if (value) {
        wrBuf[4] |= SI470X_RDS;
    } else {
        wrBuf[4] &= ~SI470X_RDS;
    }

    si470xWriteI2C(6);
}
#endif


void si470xSetPower(uint8_t value)
{
    wrBuf[1] |= SI470X_ENABLE;
    if (value) {
        wrBuf[1] &= ~SI470X_DISABLE;
    } else {
        wrBuf[1] |= SI470X_DISABLE;
    }

    si470xWriteI2C(4);
}


void si470xSeek(int8_t direction)
{
    wrBuf[0] |= SI470X_SEEK;

    if (direction > 0) {
        wrBuf[0] |= SI470X_SEEKUP;
    } else {
        wrBuf[0] &= ~SI470X_SEEKUP;
    }

    si470xWriteI2C(2);
}
