#include "rda5807.h"

#include "../i2c.h"

#define CHAN_SPACING 5

static uint8_t buf[4];

static void rda5807WriteI2C(uint8_t *buf)
{
	uint8_t i;

	I2CStart(RDA5807M_ADDR);
	for (i = 0; i < 4; i++) {
		I2CWriteByte(buf[i]);
	}
	I2CStop();

	return;
}

void rda5807Init(void)
{
	/* TODO: Init IC */
	return;
}

void rda5807SetFreq(uint16_t freq, uint8_t mono)
{
	uint16_t chan = (freq - 8700) / CHAN_SPACING;

	buf[0] = RDA5807_DHIZ | RDA5807_DMUTE | RDA5807_SEEKUP;
	if (mono)
		buf[0] |= RDA5807_MONO;

	buf[1] = RDA5807_CLK_MODE_32768;
	buf[1] |= RDA5807_ENABLE;

	buf[2] = chan >> 2;						/* 8 MSB */

	buf[3] = (chan & 0x03) << 6;			/* 2 LSB */
	buf[3] |= (RDA5807_TUNE | RDA5807_BAND_US_EUROPE | RDA5807_SPACE_50);

	rda5807WriteI2C(buf);

	return;
}

void rda5807ReadStatus(uint8_t *buf)
{
	uint8_t i;

	I2CStart(RDA5807M_ADDR | I2C_READ);
	for (i = 0; i < 4; i++)
		I2CReadByte(&buf[i], I2C_ACK);
	I2CReadByte(&buf[4], I2C_NOACK);
	I2CStop();

	return;
}
