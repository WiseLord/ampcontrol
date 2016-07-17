#include "tea5767.h"

#include "../i2c.h"
#include "../eeprom.h"
#include <avr/eeprom.h>

static uint8_t buf[5];

static uint8_t ctrl;

#define TEA5767_HCC_CTRL		(1<<6)
#define TEA5767_SNC_CTRL		(1<<5)
#define TEA5767_SMUTE_CTRL		(1<<4)
#define TEA5767_DTC_CTRL		(1<<3)
#define TEA5767_BL_CTRL			(1<<2)
#define TEA5767_PLLREF_CTRL		(1<<1)
#define TEA5767_XTAL_CTRL		(1<<0)

void tea5767Init(void)
{
	ctrl = eeprom_read_byte((uint8_t*)EEPROM_FM_CTRL);

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

	uint8_t tmpBuf;

	if (ctrl & TEA5767_XTAL_CTRL)
		div = fq / 8192;
	else
		div = fq / 12500;

	buf[0] = (div >> 8) & 0x3F;

	buf[1] = div & 0xff;

	tmpBuf = TEA5767_HLSI;
	if (mono)
		tmpBuf |= TEA5767_MS;
	buf[2] = tmpBuf;

	tmpBuf = 0;
	if (ctrl & TEA5767_HCC_CTRL)
		tmpBuf |= TEA5767_HCC;
	if (ctrl & TEA5767_SNC_CTRL)
		tmpBuf |= TEA5767_SNC;
	if (ctrl & TEA5767_SMUTE_CTRL)
		tmpBuf |= TEA5767_SMUTE;
	if (ctrl & TEA5767_BL_CTRL)
		tmpBuf |= TEA5767_BL;
	if (ctrl & TEA5767_XTAL_CTRL)
		tmpBuf |= TEA5767_XTAL;
	buf[3] = tmpBuf;

	tmpBuf = 0;
	if (ctrl & TEA5767_DTC_CTRL)
		tmpBuf |= TEA5767_DTC;
	if (ctrl & TEA5767_PLLREF_CTRL)
		tmpBuf |= TEA5767_PLLREF;
	buf[4] = tmpBuf;

	tea5767WriteI2C(buf);

	return;
}

void tea5767ReadStatus(uint8_t *buf)
{
	uint8_t i;

	I2CStart(TEA5767_ADDR | I2C_READ);
	for (i = 0; i < 4; i++)
		buf[i] = I2CReadByte(I2C_ACK);
	buf[4] = I2CReadByte(I2C_NOACK);
	I2CStop();

	return;
}
