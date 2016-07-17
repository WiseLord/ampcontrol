#include "tea5767.h"

#include "../i2c.h"

static uint8_t wrBuf[5];
static uint8_t rdBuf[5];

uint16_t div = 12500;

static void tea5767WriteI2C(void)
{
	uint8_t i;

	I2CStart(TEA5767_I2C_ADDR);
	for (i = 0; i < sizeof(wrBuf); i++)
		I2CWriteByte(wrBuf[i]);
	I2CStop();

	return;
}

void tea5767Init(uint8_t ctrl)
{
	wrBuf[0] = TEA5767_MUTE;

	wrBuf[1] = 0;

	wrBuf[2] = TEA5767_HLSI;

	wrBuf[3] = ctrl & (TEA5767_HCC | TEA5767_SNC | TEA5767_SMUTE | TEA5767_BL | TEA5767_XTAL);
	if (ctrl & TEA5767_XTAL)
		div = 8192;

	wrBuf[4] = ctrl & (TEA5767_DTC | TEA5767_PLLREF);

	return;
}

void tea5767SetFreq(uint16_t freq, uint8_t mono)
{
	uint32_t fq = (uint32_t)freq * 10000 + 225000;
	uint16_t pll = fq / div;

	wrBuf[0] &= 0xC0;
	wrBuf[0] |= (pll >> 8) & 0x3F;

	wrBuf[1] = pll & 0xFF;

	if (mono)
		wrBuf[2] |= TEA5767_MS;
	else
		wrBuf[2] &= ~TEA5767_MS;

	tea5767WriteI2C();

	return;
}

uint8_t *tea5767ReadStatus(void)
{
	uint8_t i;

	I2CStart(TEA5767_I2C_ADDR | I2C_READ);
	for (i = 0; i < sizeof(rdBuf) - 1; i++)
		rdBuf[i] = I2CReadByte(I2C_ACK);
	rdBuf[sizeof(rdBuf) - 1] = I2CReadByte(I2C_NOACK);
	I2CStop();

	return rdBuf;
}

void tea5767SetMute(uint8_t mute)
{
	if (mute)
		wrBuf[0] |= TEA5767_MUTE;
	else
		wrBuf[0] &= ~TEA5767_MUTE;

	tea5767WriteI2C();

	return;
}

void tea5767PowerOn(void)
{
	wrBuf[3] &= ~TEA5767_STBY;

	tea5767SetMute(1);

	return;
}

void tea5767PowerOff(void)
{
	wrBuf[3] |= TEA5767_STBY;

	tea5767SetMute(1);

	return;
}
