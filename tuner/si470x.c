#include "si470x.h"
#include "tuner.h"

#include "../i2c.h"
#include "../pins.h"
#include <util/delay.h>

static uint8_t band = SI470X_BAND_US_EUROPE;
static uint16_t fBL = 8750;

static void si470xWriteI2C(uint8_t bytes)
{
    uint8_t i;

    I2CStart(SI470X_I2C_ADDR);
    for (i = 0; i < bytes; i++)
        I2CWriteByte(tunerWrBuf[i]);
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
    tunerWrBuf[10] = I2CReadByte(I2C_ACK);
    tunerWrBuf[11] = I2CReadByte(I2C_NOACK);

    I2CStop();
}

void si470xInit()
{
    initRegs();

    tunerWrBuf[0] = SI470X_SKMODE;
    if (tuner.ctrl & TUNER_SMUTE) {
        tunerWrBuf[0] &= ~SI470X_DSMUTE;
    } else {
        tunerWrBuf[0] |= SI470X_DSMUTE;
    }

    if (tuner.ctrl & TUNER_DE) {
        tunerWrBuf[4] &= !SI470X_DE; // 75us used in USA
    } else {
        tunerWrBuf[4] |= SI470X_DE;  // 50us used in Europe
    }

    tunerWrBuf[5] = SI470X_BLNDADJ_19_37;

    if (tuner.ctrl & TUNER_BL) {
        fBL = 7600;
        band = SI470X_BAND_JAPAN;
    } else {
        if (tuner.fMin < 8750) {
            fBL = 7600;
            band = SI470X_BAND_JAPAN_WIDE;
        }
    }
    tunerWrBuf[7] = band | SI470X_VOLUME |
               (tuner.step2 == 20 ? SI470X_SPACE_200 : (tuner.step2 == 10 ? SI470X_SPACE_100 : SI470X_SPACE_50));

    tunerWrBuf[6] = SI470X_SEEKTH & 12; // 25 by default for backward compatibility
    tunerWrBuf[9] = (SI470X_SKSNR & 0b00010000) | (SI470X_SKCNT & 0b00000001);

    tunerWrBuf[10] |= SI470X_XOSCEN;

    tunerWrBuf[0] |= SI470X_RDSM; // New method
    tunerWrBuf[4] |= SI470X_RDS;

    si470xWriteI2C(SI470X_WRBUF_SIZE);

    if (tuner.step2 < 10)
        tuner.step2 = 5;
}

void si470xSetFreq()
{
    uint16_t chan;

    chan = (tuner.freq - fBL) / tuner.step2;

    tunerWrBuf[0] &= ~SI470X_SEEK; // not seek
    tunerWrBuf[2] = SI470X_TUNE | ((chan >> 8) & 0b00000011);
    tunerWrBuf[3] = (chan & SI470X_CHAN_7_0);

    si470xWriteI2C(4);
}

void si470xReadStatus()
{
    uint8_t i;

    I2CStart(SI470X_I2C_ADDR | I2C_READ);
    for (i = 0; i < SI470X_RDBUF_SIZE - 1; i++)
        tunerRdBuf[i] = I2CReadByte(I2C_ACK);
    tunerRdBuf[SI470X_RDBUF_SIZE - 1] = I2CReadByte(I2C_NOACK);
    I2CStop();

    if (tunerRdBuf[0] & SI740X_STC) { // seek complete
        tunerWrBuf[0] &= ~SI470X_SEEK;
        tunerWrBuf[2] &= ~SI470X_TUNE;
        si470xWriteI2C(4);
    }

    // Get RDS data
#ifdef _RDS
    if (tuner.rds) {
        // If RDS ready and sync flag is set
        if ((tunerRdBuf[0] & SI740X_RDSR) && (tunerRdBuf[0] & SI740X_RDSS)) {
            // If there are no non-correctable errors in blocks A-D
            if (    (tunerRdBuf[0] & SI740X_BLERA) != SI740X_BLERA &&
                    (tunerRdBuf[2] & SI740X_BLERB) != SI740X_BLERB &&
                    (tunerRdBuf[2] & SI740X_BLERC) != SI740X_BLERC &&
                    (tunerRdBuf[2] & SI740X_BLERD) != SI740X_BLERD ) {
                // Send rdBuf[4..11] as 16-bit blocks A-D
                rdsSetBlocks(&tunerRdBuf[4]);
            }
        }
    }
#endif

    uint16_t chan = tunerRdBuf[2] & SI740X_READCHAN_9_8;
    chan <<= 8;
    chan |= tunerRdBuf[3];

    tuner.rdFreq = chan * tuner.step2 + fBL;
}

void si470xSetVolume(int8_t value)
{
    tunerWrBuf[7] &= ~SI470X_VOLUME;
    tunerWrBuf[7] |= value;

    si470xWriteI2C(8);
}

void si470xSetMute(uint8_t value)
{
    if (value) {
        tunerWrBuf[0] &= ~SI470X_DMUTE;
    } else {
        tunerWrBuf[0] |= SI470X_DMUTE;
    }

    si470xWriteI2C(2);
}

void si470xSetMono(uint8_t value)
{
    if (value) {
        tunerWrBuf[0] |= SI470X_MONO;
    } else {
        tunerWrBuf[0] &= ~SI470X_MONO;
    }

    si470xWriteI2C(2);
}

#ifdef _RDS
void si470xSetRds(uint8_t value)
{
    rdsDisable();

    if (value) {
        tunerWrBuf[4] |= SI470X_RDS;
    } else {
        tunerWrBuf[4] &= ~SI470X_RDS;
    }

    si470xWriteI2C(6);
}
#endif


void si470xSetPower(uint8_t value)
{
    tunerWrBuf[1] |= SI470X_ENABLE;
    if (value) {
        tunerWrBuf[1] &= ~SI470X_DISABLE;
    } else {
        tunerWrBuf[1] |= SI470X_DISABLE;
    }

    si470xWriteI2C(4);
}


void si470xSeek(int8_t direction)
{
    tunerWrBuf[0] |= SI470X_SEEK;

    if (direction > 0) {
        tunerWrBuf[0] |= SI470X_SEEKUP;
    } else {
        tunerWrBuf[0] &= ~SI470X_SEEKUP;
    }

    si470xWriteI2C(2);
}
