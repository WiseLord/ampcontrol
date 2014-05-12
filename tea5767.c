#include "tea5767.h"

#include "i2c.h"

static tea5767Ctrl ctrl;

void tea5767Init(void)
{
	ctrl.high_cut = 1;
	ctrl.st_noise = 1;
	ctrl.soft_mute = 1;
	ctrl.deemph_75 = 0;
	ctrl.japan_band = 0;
	ctrl.pllref = 0;
	ctrl.xtal_freq = 1;
}

static void tea5767loadBuf(uint8_t *buf, uint16_t div)
{
	buf[0] = (div >> 8) & 0x3F;

	buf[1] = div & 0xff;

	buf[2] = TEA5767_SSL_MID;

	buf[3] = 0;
	if (ctrl.high_cut)
		buf[3] |= TEA5767_HCC;
	if (ctrl.st_noise)
		buf[3] |= TEA5767_SNC;
	if (ctrl.soft_mute)
		buf[3] |= TEA5767_SMUTE;
	if (ctrl.japan_band)
		buf[3] |= TEA5767_BL;
	if (ctrl.xtal_freq)
		buf[3] |= TEA5767_XTAL;

	buf[4] = 0;
	if (ctrl.deemph_75)
		buf[4] |= TEA5767_DTC;
	if (ctrl.pllref)
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

	div = ((uint32_t)freq * 10000 + 225000) >> 13;

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
	ret <<= 13;
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
		div = ((uint32_t)freq * 10000 + 100000 + 225000) >> 13;
//		ctrl.japan_band = 0;
	} else {
		div = ((uint32_t)freq * 10000 - 100000 + 225000) >> 13;
//		ctrl.japan_band = 1;
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

void fineTune(uint16_t *freqFM, uint8_t *bufFM)
{
	*freqFM = tea5767FreqAvail(bufFM);

	if (*freqFM > (uint32_t)FM_FREQ_MAX)
		*freqFM = (uint32_t)FM_FREQ_MIN;
	if (*freqFM < (uint32_t)FM_FREQ_MIN)
		*freqFM = (uint32_t)FM_FREQ_MAX;

	tea5767SetFreq(*freqFM);
}
