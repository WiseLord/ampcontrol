#include "tea5767.h"

#include "../i2c.h"
#include "../eeprom.h"
#include <avr/eeprom.h>

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
	ctrl = eeprom_read_byte(eepromFMCtrl);
}

static void tea5767loadBuf(uint8_t *buf, uint16_t div)
{
	buf[0] = (div >> 8) & 0x3F;

	buf[1] = div & 0xff;

	buf[2] = TEA5767_SSL_MID;

	buf[3] = 0;
	if (ctrl & TEA5767_HCC_CTRL)
		buf[3] |= TEA5767_HCC;
	if (ctrl & TEA5767_SNC_CTRL)
		buf[3] |= TEA5767_SNC;
	if (ctrl & TEA5767_SMUTE_CTRL)
		buf[3] |= TEA5767_SMUTE;
	if (ctrl & TEA5767_BL_CTRL)
		buf[3] |= TEA5767_BL;
	if (ctrl & TEA5767_XTAL_CTRL)
		buf[3] |= TEA5767_XTAL;

	buf[4] = 0;
	if (ctrl & TEA5767_DTC_CTRL)
		buf[4] |= TEA5767_DTC;
	if (ctrl & TEA5767_PLLREF_CTRL)
		buf[4] |= TEA5767_PLLREF;
}

static void tea5767WriteI2C(uint8_t *buf)
{
	uint8_t i;

	I2CStart(TEA5767_ADDR);
	for (i = 0; i < 5; i++) {
		I2CWriteByte(buf[i]);
	}
	I2CStop();

}

void tea5767SetFreq(uint16_t freq)
{
	uint8_t buf[5];
	uint16_t div;

	if (ctrl & TEA5767_XTAL_CTRL)
		div = ((uint32_t)freq * 10000 + 225000) >> 13;
	else
		div = ((uint32_t)freq * 10000 + 225000) / 12500;


	tea5767loadBuf(buf, div);

	buf[2] |= TEA5767_HLSI;

	tea5767WriteI2C(buf);

	return;
}

void tea5767ReadStatus(uint8_t *buf)
{
	uint8_t i;

	I2CStart(TEA5767_ADDR | I2C_READ);
	for (i = 0; i < 5; i++)
		I2CReadByte(&buf[i], 1);
	I2CStop();

}

uint8_t tea5767ADCLevel(uint8_t *buf)
{
	return (buf[3] & TEA5767_LEV_MASK) >> 4;
}

uint16_t tea5767FreqAvail(uint8_t *buf)
{
	uint32_t ret;

	ret = buf[0];
	ret &= 0x3F;
	ret <<= 8;
	ret += buf[1];
	if (ctrl & TEA5767_XTAL_CTRL) {
		ret <<= 13;
	} else {
		ret *= 12500;
	}
	ret -= 225000;
	ret += 25000;
	ret /= 50000;
	ret *= 5;

	return (uint16_t)ret;
}

void tea5767Search(uint16_t freq, uint8_t *buf, uint8_t direction)
{
	uint16_t div;

	if (direction == SEARCH_UP) {
		if (ctrl & TEA5767_XTAL_CTRL)
			div = ((uint32_t)freq * 10000 + 100000 + 225000) >> 13;
		else
			div = ((uint32_t)freq * 10000 + 100000 + 225000) / 12500;
	} else {
		if (ctrl & TEA5767_XTAL_CTRL)
			div = ((uint32_t)freq * 10000 - 100000 + 225000) >> 13;
		else
			div = ((uint32_t)freq * 10000 + 100000 + 225000) / 12500;
	}

	tea5767loadBuf(buf, div);

	buf[0] |= TEA5767_SM;
	buf[0] |= TEA5767_MUTE;
	buf[2] |= TEA5767_HLSI;
	if (direction == SEARCH_UP)
		buf[2] |= TEA5767_SUD;

	tea5767WriteI2C(buf);

	return;
}
