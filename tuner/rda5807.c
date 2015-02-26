#include "rda5807.h"

#include "../i2c.h"

static uint8_t wrBuf[8];
static uint8_t rdBuf[5];

static void rda5807WriteI2C(void)
{
	uint8_t i;

	I2CStart(RDA5807M_I2C_ADDR);
	for (i = 0; i < sizeof(wrBuf); i++)
		I2CWriteByte(wrBuf[i]);
	I2CStop();

	return;
}

void rda5807Init(void)
{
	wrBuf[0] = RDA5807_DHIZ;
	wrBuf[1] = RDA5807_CLK_MODE_32768 | RDA5807_NEW_METHOD | RDA5807_ENABLE;
	wrBuf[2] = 0;
	wrBuf[3] = RDA5807_BAND | RDA5807_SPACE;
	wrBuf[4] = 0;
	wrBuf[5] = 0;
	wrBuf[6] = 0b1000 & RDA5807_SEEKTH;
	wrBuf[7] = 0b10000000 & RDA5807_LNA_PORT_SEL;

	rda5807WriteI2C();

	return;
}

void rda5807SetFreq(uint16_t freq, uint8_t mono)
{
	uint16_t chan = (freq - RDA5807_FREQ_MIN) / RDA5807_CHAN_SPACING;

	if (mono)
		wrBuf[0] |= RDA5807_MONO;
	else
		wrBuf[0] &= ~RDA5807_MONO;

	wrBuf[2] = chan >> 2;								/* 8 MSB */

	wrBuf[3] &= 0x3F;
	wrBuf[3] |= RDA5807_TUNE | ((chan & 0x03) << 6);	/* 2 LSB */

	rda5807WriteI2C();

	return;
}

void rda5807SetVolume(int8_t value)
{
	wrBuf[3] &= ~RDA5807_TUNE;

	if (value) {
		wrBuf[7] &= 0xF0;
		wrBuf[7] |= (value - 1);
		wrBuf[0] |= RDA5807_DMUTE;
	} else {
		wrBuf[0] &= ~RDA5807_DMUTE;
	}

	rda5807WriteI2C();

	return;
}

void rda5807MuteVolume(void)
{
	wrBuf[0] &= ~RDA5807_DMUTE;

	rda5807WriteI2C();

	return;
}

void rda5807UnmuteVolume(void)
{
	wrBuf[0] |= RDA5807_DMUTE;

	rda5807WriteI2C();

	return;
}

uint8_t *rda5807ReadStatus(void)
{
	uint8_t i;

	I2CStart(RDA5807M_I2C_ADDR | I2C_READ);
	for (i = 0; i < sizeof(rdBuf) - 1; i++)
		I2CReadByte(&rdBuf[i], I2C_ACK);
	I2CReadByte(&rdBuf[sizeof(rdBuf) - 1], I2C_NOACK);
	I2CStop();

	return rdBuf;
}

