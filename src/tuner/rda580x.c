#include "rda580x.h"
#include "tuner.h"

#include "../i2c.h"

#ifdef _RDS
#include "rds.h"
#endif

static uint8_t band = RDA580X_BAND_US_EUROPE;
static uint16_t fBL = 8700;

static void rda580xWriteReg(uint8_t reg)
{
    uint8_t *wrAddr = &tunerWrBuf[2 * reg - 4];

    I2CStart(RDA5807M_I2C_RAND_ADDR);
    I2CWriteByte(reg);
    I2CWriteByte(*wrAddr++);
    I2CWriteByte(*wrAddr++);
    I2CStop();
}

static void rda580xSetBit(uint8_t idx, uint8_t bit, uint8_t cond)
{
    if (cond) {
        tunerWrBuf[idx] |= bit;
    } else {
        tunerWrBuf[idx] &= ~bit;
    }

    rda580xWriteReg(idx / 2 + 2);
}

void rda580xInit(void)
{
    tunerWrBuf[0] = RDA580X_DHIZ;
    tunerWrBuf[1] = RDA580X_SKMODE | RDA580X_CLK_MODE_32768 | RDA5807_NEW_METHOD;
    tunerWrBuf[6] = 0x08 & RDA5807_SEEKTH;
    tunerWrBuf[7] = RDA580X_LNA_PORT_SEL_LNAP | RDA580X_VOLUME;
#ifdef _RDA5807_DF
    tunerWrBuf[10] = (0x40 & RDA5807_TH_SOFRBLEND);
#endif

    rda580xSetMono(tuner.mono);
    rda580xSetBass(tuner.bass);
#ifdef _RDS
    rda580xSetRds(tuner.rds);
#endif
#ifdef _RDA5807_DF
    if (tuner.ic == TUNER_RDA5807 && (tuner.ctrl & TUNER_DFREQ)) {
        tunerWrBuf[11] |= RDA5807_FREQ_MODE;
    } else {
        tunerWrBuf[11] &= RDA5807_FREQ_MODE;
    }
    rda580xWriteReg(7);
#endif
    if (tuner.ctrl & TUNER_DE) {
        tunerWrBuf[4] &= ~RDA580X_DE;
    } else {
        tunerWrBuf[4] |= RDA580X_DE;
    }

    if (tuner.ctrl & TUNER_SMUTE) {
        tunerWrBuf[4] |= RDA580X_SOFTMUTE_EN;
    } else {
        tunerWrBuf[4] &= ~RDA580X_SOFTMUTE_EN;
    }
    rda580xWriteReg(4);

    if (tuner.ctrl & TUNER_BL) {
        fBL = 7600;
        band = RDA580X_BAND_JAPAN;
    } else {
        if (tuner.fMin < 8700) {
            fBL = 7600;
            band = RDA580X_BAND_WORLDWIDE;
        }
    }

    if (tuner.step2 < 10)
        tuner.step2 = 5;
}

void rda580xSetFreq(void)
{
#ifdef _RDA5807_DF
    if (tunerWrBuf[11] & RDA5807_FREQ_MODE) {
        uint16_t df = (tuner.freq - 5000) * 10;
        tunerWrBuf[13] = df & 0xFF;
        tunerWrBuf[12] = df >> 8;
        rda580xWriteReg(8);

        tunerWrBuf[3] = RDA580X_TUNE | RDA580X_BAND_EASTEUROPE;
    } else {
#else
    {
#endif
        // Freq in grid
        uint16_t chan = (tuner.freq - fBL) / tuner.step2;
        tunerWrBuf[2] = ((chan >> 2) & RDA580X_CHAN_9_2);      // 8 MSB
        tunerWrBuf[3] = ((chan << 6) & RDA580X_CHAN_1_0) | RDA580X_TUNE | band |
        (tuner.step2 == 20 ? RDA580X_SPACE_200 : (tuner.step2 == 10 ? RDA580X_SPACE_100 : RDA580X_SPACE_50));
    }
    rda580xWriteReg(2); // Stop seek if it is
    rda580xWriteReg(3);
}

void rda580xReadStatus(void)
{
    uint8_t i;

    I2CStart(RDA5807M_I2C_SEQ_ADDR | I2C_READ);
    for (i = 0; i < RDA5807_RDBUF_SIZE - 1; i++)
        tunerRdBuf[i] = I2CReadByte(I2C_ACK);
    tunerRdBuf[RDA5807_RDBUF_SIZE - 1] = I2CReadByte(I2C_NOACK);
    I2CStop();

    // Get RDS data
#ifdef _RDS
    if (tuner.rds) {
        // If RDS ready and sync flag are set
        if ((tunerRdBuf[0] & RDA5807_RDSR) && (tunerRdBuf[0] & RDA5807_RDSS)) {
            // If there are no non-correctable errors in blocks A-D
            if (    (tunerRdBuf[3] & RDA5807_BLERA) != RDA5807_BLERA &&
                    (tunerRdBuf[3] & RDA5807_BLERB) != RDA5807_BLERB ) {
                // Send rdBuf[4..11] as 16-bit blocks A-D
                rdsSetBlocks(&tunerRdBuf[4]);
            }
        }
    }
#endif
    uint16_t chan = tunerRdBuf[0] & RDA580X_READCHAN_9_8;
    chan <<= 8;
    chan |= tunerRdBuf[1];

#ifdef _RDA5807_DF
    if (tunerWrBuf[11] & RDA5807_FREQ_MODE) {
        tuner.rdFreq = tuner.freq;
    } else {
#else
    {
#endif
        uint16_t chan = tunerRdBuf[0] & RDA580X_READCHAN_9_8;
        chan <<= 8;
        chan |= tunerRdBuf[1];
        tuner.rdFreq = chan * tuner.step2 + fBL;
    }
}

void rda580xSetVolume(int8_t value)
{
    if (value)
        tunerWrBuf[7] = RDA580X_LNA_PORT_SEL_LNAP | (value - 1);
    else
        rda580xSetMute(1);

    rda580xWriteReg(5);
}

void rda580xSetMute(uint8_t value)
{
    rda580xSetBit(0, RDA580X_DMUTE, !value);
}

void rda580xSetBass(uint8_t value)
{
    rda580xSetBit(0, RDA5807_BASS, value);
}

void rda580xSetMono(uint8_t value)
{
    rda580xSetBit(0, RDA580X_MONO, value);
}

#ifdef _RDS
void rda580xSetRds(uint8_t value)
{
    rdsDisable();

    rda580xSetBit(1, RDA5807_RDS_EN, value);
}
#endif

void rda580xSetPower(uint8_t value)
{
    rda580xSetBit(1, RDA580X_ENABLE, value);
}

void rda580xSeek(int8_t direction)
{
#ifdef _RDA5807_DF
    if (tunerWrBuf[11] & RDA5807_FREQ_MODE) {
        tunerChangeFreq(direction);
        return;
    }
#endif
    tunerWrBuf[0] |= RDA580X_SEEK;
    rda580xSetBit(0, RDA580X_SEEKUP, direction > 0);
    tunerWrBuf[0] &= ~RDA580X_SEEK;
}
