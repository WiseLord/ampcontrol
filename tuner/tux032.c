#include "tux032.h"

#include "../i2c.h"

static uint8_t wrBuf[9] = {0x80, 0x00, 0x00, 0x64, 0xB1, 0xC6, 0x4B, 0xA2, 0xD2};
static uint8_t rdBuf[4];

static void tux032WriteI2C(uint8_t bytes)
{
	uint8_t i;

	I2CStart(TUX032_I2C_ADDR);
	for (i = 0; i < bytes; i++)
		I2CWriteByte(wrBuf[i]);
	I2CStop();

	return;
}

void tux032Init(void)
{
	tux032PowerOff();

	return;
}

void tux032SetFreq(uint16_t freq)
{
	freq = freq / 5 + 214;

	wrBuf[0] = 0x80;
	wrBuf[1] = freq >> 8;
	wrBuf[2] = freq & 0xFF;

	tux032WriteI2C(sizeof(wrBuf));

	return;
}

void tux032PowerOn(void)
{
	wrBuf[0] = 0x82;
	wrBuf[1] = 0x64;

	tux032WriteI2C(2);

	return;
}

void tux032PowerOff(void)
{
	wrBuf[0] = 0x82;
	wrBuf[1] = 0x00;

	tux032WriteI2C(2);

	return;
}

uint8_t *tux032ReadStatus(void)
{
	uint8_t i;

	I2CStart(TUX032_I2C_ADDR | I2C_READ);
	for (i = 0; i < sizeof(rdBuf) - 1; i++)
		I2CReadByte(&rdBuf[i], I2C_ACK);
	I2CReadByte(&rdBuf[sizeof(rdBuf) - 1], I2C_NOACK);
	I2CStop();

	return rdBuf;
}
