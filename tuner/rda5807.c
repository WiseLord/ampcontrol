#include "rda5807.h"

#include "../i2c.h"

void rda5807Init(void)
{
	/* TODO: Init IC */
	return;
}

static void rda5807WriteI2C(uint8_t *buf)
{
	uint8_t i;

	I2CStart(RDA5807M_ADDR);
	for (i = 0; i < 14; i++) {
		I2CWriteByte(buf[i]);
	}
	I2CStop();

	return;
}

void rda5807SetFreq(uint16_t freq, uint8_t mono)
{
	uint8_t buf[14];

	/* TODO: Calculate setup word and set frequency */


	rda5807WriteI2C(buf);

	return;
}

void rda5807ReadStatus(uint8_t *buf)
{
	uint8_t i;

	I2CStart(RDA5807M_ADDR | I2C_READ);
	for (i = 0; i < 12; i++)
		I2CReadByte(&buf[i], 1);
	I2CStop();

	return;
}
