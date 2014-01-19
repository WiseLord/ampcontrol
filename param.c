#include "param.h"

#include <avr/eeprom.h>
#include <util/delay.h>

#include "ks0108.h"
#include "i2c.h"
#include "eeprom.h"

regParam *params[10] = {
	&volume,
	&bass,
	&middle,
	&treble,
	&preamp,
	&balance,
	&gain[0],
	&gain[1],
	&gain[2],
	&gain[3],
};

void setPreamp(int8_t val)
{
	I2CWrite(TDA7439_ADDR, FUNC_PREAMP, -val);
}

void setVolume(int8_t val)
{
	int8_t spLeft = val;
	int8_t spRight = val;

	if (balance.value > 0) {
		spLeft -= balance.value;
		if (spLeft < volume.min)
			spLeft = volume.min;
	} else {
		spRight += balance.value;
		if (spRight < volume.min)
			spRight = volume.min;
	}

	I2CWrite(TDA7439_ADDR, FUNC_VOLUME_LEFT, -spLeft);
	I2CWrite(TDA7439_ADDR, FUNC_VOLUME_RIGHT, -spRight);
}

int8_t setBMT(int8_t val)
{
	if (val > 0)
		return 15 - val;
	return 7 + val;
}

void setBass(int8_t val)
{
	I2CWrite(TDA7439_ADDR, FUNC_BASS, setBMT(val));
}

void setMiddle(int8_t val)
{
	I2CWrite(TDA7439_ADDR, FUNC_MIDDLE, setBMT(val));
}

void setTreble(int8_t val)
{
	I2CWrite(TDA7439_ADDR, FUNC_TREBLE, setBMT(val));
}

void setGain(int8_t val)
{
	I2CWrite(TDA7439_ADDR, FUNC_INPUT_GAIN, val);
}

void setChan(uint8_t ch)
{
	setGain(gain[ch].value);
	I2CWrite(TDA7439_ADDR, FUNC_INPUT_SELECT, 3 - ch);
	chan = ch;
}

void setBalance(int8_t val)
{
	setVolume(volume.value);
}

void muteVolume(void) {
	setVolume(volume.min);
}

void unmuteVolume(void) {
	setVolume(volume.value);
}

void loadParams(void)
{
	uint8_t i;

	for (i = 0; i < 10; i++) {
		params[i]->value = eeprom_read_byte(eepromVolume + i);
		params[i]->label = volumeLabel + 16 * i;
		params[i]->min = eeprom_read_byte(eepromMinimums + i);
		params[i]->max = eeprom_read_byte(eepromMaximums + i);
		params[i]->step = eeprom_read_byte(eepromSteps + i);
	}

	chan = eeprom_read_byte(eepromChannel);

	volume.set = setVolume;
	bass.set = setBass;
	middle.set = setMiddle;
	treble.set = setTreble;
	balance.set = setBalance;
	preamp.set = setPreamp;

	for (i = 0; i < 4; i++) {
		gain[i].set = setGain;
	}

	setChan(chan);
	setPreamp(preamp.value);
	setBass(bass.value);
	setMiddle(middle.value);
	setTreble(treble.value);
}

void saveParams(void)
{
	uint8_t i;

	for (i = 0; i < 10; i++) {
		eeprom_write_byte(eepromVolume + i, params[i]->value);
	}
}

void incParam(regParam *param)
{
	param->value++;
	if (param->value > param->max)
		param->value = param->max;
	param->set(param->value);
}

void decParam(regParam *param)
{
	param->value--;
	if (param->value < param->min)
		param->value = param->min;
	param->set(param->value);
}

void nextChan(void)
{
	chan++;
	if (chan >= 4)
		chan = 0;
	setChan(chan);
}

void showParLabel(const uint8_t *parLabel)
{
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteStringEeprom(parLabel);
	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(116, 7);
	gdWriteStringEeprom(dbLabel);
}

void showParValue(int8_t value)
{
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(93, 4);
	gdWriteString(mkNumString(value, 3, ' '));
}

void showBar(uint8_t length, int8_t from, int8_t to)
{
	int8_t i, j;
	uint8_t data;
	for (j = 5; j <=6; j++) {
		gdSetXY(0, j);
		for (i = 0; i < length; i++) {
			if (j == 5)
				data = 0x80;
			else
				data = 0x01;
			if (i >= from && i <= to)
				data = 0xFF;
			if (i % 2)
				data = 0x00;
			gdWriteData(data);
		}
	}
}

void showParam(regParam *param)
{
	int8_t l, r, m;
	m = 94 / (param->max - param->min);
	if (param->min < 0 && param->max > 0) {
		l = 42;
		r = 42;
		if (param->value > 0) {
			r += m * param->value;
		} else {
			l += m * param->value;
		}
		showBar(86, l, r);
	} else {
		l = 0;
		r = m * (param->value - param->min) - 1;
		showBar(m * (param->max - param->min), l, r);
	}
	showParValue(param->value * param->step);
	showParLabel(param->label);
}
