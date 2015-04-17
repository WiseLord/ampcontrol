#include "tea5767.h"

#include "../i2c.h"

static uint8_t wrBuf[5];
static uint8_t rdBuf[5];

static uint8_t ctrl = 0x79;

#define TEA5767_CTRL_HCC		(1<<6)
#define TEA5767_CTRL_SNC		(1<<5)
#define TEA5767_CTRL_SMUTE		(1<<4)
#define TEA5767_CTRL_DTC		(1<<3)
#define TEA5767_CTRL_BL			(1<<2)
#define TEA5767_CTRL_PLLREF		(1<<1)
#define TEA5767_CTRL_XTAL		(1<<0)

static void tea5767WriteI2C(void)
{
	uint8_t i;

	I2CStart(TEA5767_I2C_ADDR);
	for (i = 0; i < sizeof(wrBuf); i++)
		I2CWriteByte(wrBuf[i]);
	I2CStop();

	return;
}

void tea5767Init(uint8_t tea5767Ctrl)
{
	ctrl = tea5767Ctrl;

	wrBuf[0] = TEA5767_MUTE;

	wrBuf[1] = 0;

	wrBuf[2] = TEA5767_HLSI;

	wrBuf[3] = 0;
	if (ctrl & TEA5767_CTRL_HCC)
		wrBuf[3] |= TEA5767_HCC;
	if (ctrl & TEA5767_CTRL_SNC)
		wrBuf[3] |= TEA5767_SNC;
	if (ctrl & TEA5767_CTRL_SMUTE)
		wrBuf[3] |= TEA5767_SMUTE;
	if (ctrl & TEA5767_CTRL_BL)
		wrBuf[3] |= TEA5767_BL;
	if (ctrl & TEA5767_CTRL_XTAL)
		wrBuf[3] |= TEA5767_XTAL;

	wrBuf[4] = 0;
	if (ctrl & TEA5767_CTRL_DTC)
		wrBuf[4] |= TEA5767_DTC;
	if (ctrl & TEA5767_CTRL_PLLREF)
		wrBuf[4] |= TEA5767_PLLREF;

	return;
}

void tea5767SetFreq(uint16_t freq, uint8_t mono)
{
	uint16_t div;

	uint32_t fq = (uint32_t)freq * 10000 + 225000;

	if (ctrl & TEA5767_CTRL_XTAL)
		div = fq / 8192;
	else
		div = fq / 12500;

	wrBuf[0] &= 0xC0;
	wrBuf[0] |= (div >> 8) & 0x3F;

	wrBuf[1] = div & 0xFF;

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
		I2CReadByte(&rdBuf[i], I2C_ACK);
	I2CReadByte(&rdBuf[sizeof(rdBuf) - 1], I2C_NOACK);
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

	tea5767SetMute(0);

	return;
}

void tea5767PowerOff(void)
{
	wrBuf[3] |= TEA5767_STBY;

	tea5767SetMute(1);

	return;
}
