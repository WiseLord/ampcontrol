#include "rda580x.h"
#include "tuner.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

#ifdef _RDS
#include "rds.h"
#endif

static uint8_t wrBuf[14] = {
	RDA580X_DHIZ | RDA580X_DMUTE,
	RDA580X_CLK_MODE_32768 | RDA5807_NEW_METHOD | RDA580X_ENABLE,
	0,
	RDA580X_BAND_EASTEUROPE | RDA580X_SPACE_50,
	0,
	0,
	0b1000 & RDA5807_SEEKTH,
	RDA580X_LNA_PORT_SEL_LNAP | RDA580X_VOLUME,
	0,
	0,
	(0x80 & RDA5807_TH_SOFRBLEND),
	0,
	0,
	0,
};
static uint8_t rdBuf[12];

static void rda580xWriteI2C(uint8_t bytes)
{
	uint8_t i;

	if (tuner.ic == TUNER_RDA5802)
		bytes = RDA5802_WR_BYTES;

	I2CStart(RDA5807M_I2C_ADDR);
	for (i = 0; i < bytes; i++)
		I2CWriteByte(wrBuf[i]);
	I2CStop();

	return;
}

void rda580xInit(void)
{
	if (tuner.ic == TUNER_RDA5807_DF)
		wrBuf[11] |= RDA5807_FREQ_MODE;

	return;
}

void rda580xSetFreq(void)
{
	uint16_t chan;
	uint16_t fMin = RDA5807_MIN_FREQ;
	uint8_t band = RDA580X_BAND_EASTEUROPE;

	if (tuner.mono)
		wrBuf[0] |= RDA580X_MONO;
	else
		wrBuf[0] &= ~RDA580X_MONO;

#ifdef _RDS
	if (tuner.rds)
		wrBuf[1] |= RDA5807_RDS_EN;
	else
		wrBuf[0] &= ~RDA5807_RDS_EN;
#endif

	if (tuner.ic == TUNER_RDA5802)
		fMin = RDA5802_MIN_FREQ;

	if (tuner.freq >= RDA5807_BAND_CHANGE_FREQ) {
		fMin = RDA5807_BAND_CHANGE_FREQ;
		band = RDA580X_BAND_US_EUROPE;
	}

	// Freq in grid
	chan = (tuner.freq - fMin) / RDA5807_CHAN_SPACING;
	wrBuf[2] = chan >> 2;								/* 8 MSB */
	wrBuf[3] = ((chan & 0x03) << 6) | RDA580X_TUNE | band | RDA580X_SPACE_50;

	// Direct freq
	wrBuf[12] = ((tuner.freq - fMin) * 10) >> 8;
	wrBuf[13] = ((tuner.freq - fMin) * 10) & 0xFF;

	rda580xWriteI2C(RDA5807_WR_BYTES);

	return;
}

uint8_t *rda580xReadStatus(void)
{
	uint8_t i;

	I2CStart(RDA5807M_I2C_ADDR | I2C_READ);
	for (i = 0; i < sizeof(rdBuf) - 1; i++)
		rdBuf[i] = I2CReadByte(I2C_ACK);
	rdBuf[sizeof(rdBuf) - 1] = I2CReadByte(I2C_NOACK);
	I2CStop();

	// Get RDS data
#ifdef _RDS
	if (tuner.rds) {
		/* If seek/tune is complete and current channel is a station */
//		if ((rdBuf[0] & RDA580X_STC) && (rdBuf[2] & RDA580X_FM_TRUE)) {
			/* If RDS ready and sync flag are set */
			if ((rdBuf[0] & RDA5807_RDSR) && (rdBuf[0] & RDA5807_RDSS)) {
				/* If there are no errors in blocks A and B */
				if (!(rdBuf[3] & (RDA5807_BLERA | RDA5807_BLERB))) {
					/* Send rdBuf[4..11] as 16-bit blocks A-D */
					rdsSetBlocks(&rdBuf[4]);
				}
			}
//		}
	}
#endif

	return rdBuf;
}

void rda580xSetAudio(void)
{
	if (tuner.volume > RDA5807_VOL_MAX)
		tuner.volume = RDA5807_VOL_MAX;

	if (tuner.mute || !tuner.volume)
		wrBuf[0] &= ~RDA580X_DMUTE;
	else
		wrBuf[0] |= RDA580X_DMUTE;

	if (tuner.bass)
		wrBuf[0] |= RDA5807_BASS;
	else
		wrBuf[0] &= ~RDA5807_BASS;

	wrBuf[3] &= ~RDA580X_TUNE;

	if (tuner.volume)
		wrBuf[7] = RDA580X_LNA_PORT_SEL_LNAP | (tuner.volume - 1);

	rda580xWriteI2C(RDA5802_WR_BYTES);
}

void rda580xPowerOn(void)
{
	wrBuf[1] |= RDA580X_ENABLE;

	return;
}

void rda580xPowerOff(void)
{
	wrBuf[1] &= ~RDA580X_ENABLE;

	rda580xWriteI2C(RDA5802_WR_BYTES);

	return;
}
