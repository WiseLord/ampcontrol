#include "tux032.h"

#include "../i2c.h"

void tux032Init(void)
{
	tux032GoStby();
	return;
}

void tux032SetFreq(uint16_t freq)
{
	freq = freq / 5 + 214;

	I2CStart(TUX032_ADDR);
	I2CWriteByte(0x80);
	I2CWriteByte((freq & 0xFF00) >> 8);
	I2CWriteByte(freq & 0x00FF);
	I2CWriteByte(0x64);
	I2CWriteByte(0xB1);
	I2CWriteByte(0xC6);
	I2CWriteByte(0x4B);
	I2CWriteByte(0xA2);
	I2CWriteByte(0xD2);

	I2CStop();

	return;
}

void tux032GoStby()
{
	I2CStart(TUX032_ADDR);
	I2CWriteByte(0x82);
	I2CWriteByte(0x00);

	I2CStop();

	return;
}

void tux032ExitStby()
{
	I2CStart(TUX032_ADDR);
	I2CWriteByte(0x82);
	I2CWriteByte(0x64);

	I2CStop();

	return;
}

void tux032ReadStatus(uint8_t *buf)
{
	uint8_t i;

	I2CStart(TUX032_ADDR | I2C_READ);
	for (i = 0; i < 4; i++)
		I2CReadByte(&buf[i], I2C_ACK);
	I2CReadByte(&buf[4], I2C_NOACK);
	I2CStop();

	return;
}
