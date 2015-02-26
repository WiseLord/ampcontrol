#include "tea5767.h"

#include "../i2c.h"

static uint8_t wrBuf[5];
static uint8_t rdBuf[5];

static uint8_t ctrl = 0x71;

#define TEA5767_HCC_CTRL		(1<<6)
#define TEA5767_SNC_CTRL		(1<<5)
#define TEA5767_SMUTE_CTRL		(1<<4)
#define TEA5767_DTC_CTRL		(1<<3)
#define TEA5767_BL_CTRL			(1<<2)
#define TEA5767_PLLREF_CTRL		(1<<1)
#define TEA5767_XTAL_CTRL		(1<<0)

void tea5767Init(uint8_t tea5767Ctrl)
{
	ctrl = tea5767Ctrl;

	return;
}

static void tea5767WriteI2C(uint8_t *buf)
{
	uint8_t i;

	I2CStart(TEA5767_ADDR);
	for (i = 0; i < 5; i++) {
		I2CWriteByte(buf[i]);
	}
	I2CStop();

	return;
}

void tea5767SetFreq(uint16_t freq, uint8_t mono)
{
	uint16_t div;

	uint32_t fq = (uint32_t)freq * 10000 + 225000;

	if (ctrl & TEA5767_XTAL_CTRL)
		div = fq / 8192;
	else
		div = fq / 12500;

	wrBuf[0] = (div >> 8) & 0x3F;

	wrBuf[1] = div & 0xff;

	wrBuf[2] = TEA5767_HLSI;
	if (mono)
		wrBuf[2] |= TEA5767_MS;

	wrBuf[3] = 0;
	if (ctrl & TEA5767_HCC_CTRL)
		wrBuf[3] |= TEA5767_HCC;
	if (ctrl & TEA5767_SNC_CTRL)
		wrBuf[3] |= TEA5767_SNC;
	if (ctrl & TEA5767_SMUTE_CTRL)
		wrBuf[3] |= TEA5767_SMUTE;
	if (ctrl & TEA5767_BL_CTRL)
		wrBuf[3] |= TEA5767_BL;
	if (ctrl & TEA5767_XTAL_CTRL)
		wrBuf[3] |= TEA5767_XTAL;

	wrBuf[4] = 0;
	if (ctrl & TEA5767_DTC_CTRL)
		wrBuf[4] |= TEA5767_DTC;
	if (ctrl & TEA5767_PLLREF_CTRL)
		wrBuf[4] |= TEA5767_PLLREF;

	tea5767WriteI2C(wrBuf);

	return;
}

uint8_t *tea5767ReadStatus(void)
{
	uint8_t i;

	I2CStart(TEA5767_ADDR | I2C_READ);
	for (i = 0; i < sizeof(rdBuf) - 1; i++)
		I2CReadByte(&rdBuf[i], I2C_ACK);
	I2CReadByte(&rdBuf[sizeof(rdBuf) - 1], I2C_NOACK);
	I2CStop();

	return rdBuf;
}
