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

void setVolume(int8_t val)
{
	int8_t spFrontLeft = 0;
	int8_t spFrontRight = 0;
	int8_t spRearLeft = 0;
	int8_t spRearRight = 0;

	switch (tdaIC) {
	case TDA7313_IC:
		I2CWrComm(TDA7313_ADDR, TDA7313_VOLUME | -val);
		if (balance.value > 0) {
			spFrontRight -= balance.value;
			spRearRight -= balance.value;
		} else {
			spFrontLeft += balance.value;
			spRearLeft += balance.value;
		}
		if (preamp.value > 0) {
			spRearLeft -= preamp.value;
			spRearRight -= preamp.value;
		} else {
			spFrontLeft += preamp.value;
			spFrontRight += preamp.value;
		}
		I2CWrComm(TDA7313_ADDR, TDA7313_SP_FRONT_LEFT | -spFrontLeft);
		I2CWrComm(TDA7313_ADDR, TDA7313_SP_FRONT_RIGHT | -spFrontRight);
		I2CWrComm(TDA7313_ADDR, TDA7313_SP_REAR_LEFT | -spRearLeft);
		I2CWrComm(TDA7313_ADDR, TDA7313_SP_REAR_RIGHT | -spRearRight);
		break;
	default:
		spFrontLeft = val;
		spFrontRight = val;
		if (balance.value > 0) {
			spFrontLeft -= balance.value;
			if (spFrontLeft < volume.min)
				spFrontLeft = volume.min;
		} else {
			spFrontRight += balance.value;
			if (spFrontRight < volume.min)
				spFrontRight = volume.min;
		}
		I2CWrite(TDA7439_ADDR, TDA7439_VOLUME_LEFT, -spFrontLeft);
		I2CWrite(TDA7439_ADDR, TDA7439_VOLUME_RIGHT, -spFrontRight);
		break;
	}
}

void setPreamp(int8_t val) /* For TDA7313 used as balance front/rear */
{
	switch (tdaIC) {
	case TDA7313_IC:
		setVolume(volume.value);
		break;
	default:
		I2CWrite(TDA7439_ADDR, TDA7439_PREAMP, -val);
		break;
	}
}

int8_t setBMT(int8_t val)
{
	if (val > 0)
		return 15 - val;
	return 7 + val;
}

void setBass(int8_t val)
{
	switch (tdaIC) {
	case TDA7313_IC:
		I2CWrComm(TDA7313_ADDR, TDA7313_BASS | setBMT(val));
		break;
	default:
		I2CWrite(TDA7439_ADDR, TDA7439_BASS, setBMT(val));
		break;
	}
}

void setMiddle(int8_t val)
{
	switch (tdaIC) {
	case TDA7313_IC:
		break;
	default:
		I2CWrite(TDA7439_ADDR, TDA7439_MIDDLE, setBMT(val));
		break;
	}
}

void setTreble(int8_t val)
{
	switch (tdaIC) {
	case TDA7313_IC:
		I2CWrComm(TDA7313_ADDR, TDA7313_TREBLE | setBMT(val));
		break;
	default:
		I2CWrite(TDA7439_ADDR, TDA7439_TREBLE, setBMT(val));
		break;
	}
}

void setSwitch(int8_t gain)
{
	I2CWrComm(TDA7313_ADDR, TDA7313_SW | (3 - gain) << 3 | loud << 2 | chan);
}

void setGain(int8_t val)
{
	switch (tdaIC) {
	case TDA7313_IC:
		setSwitch(val);
		break;
	default:
		I2CWrite(TDA7439_ADDR, TDA7439_INPUT_GAIN, val);
		break;
	}
}

void setChan(uint8_t ch)
{
	chan = ch;
	setGain(gain[ch].value);
	switch (tdaIC) {
	case TDA7313_IC:
		setSwitch(gain[chan].value);
		break;
	default:
		I2CWrite(TDA7439_ADDR, TDA7439_INPUT_SELECT, chanCnt - ch);
		break;
	}
}

void setBalance(int8_t val)
{
	setVolume(volume.value);
}

void muteVolume(void)
{
	setVolume(volume.min);
}

void unmuteVolume(void)
{
	setVolume(volume.value);
}

void switchLoudness(void)
{
	if (loud == 0)
		loud = 1;
	else
		loud = 0;
	setSwitch(gain[chan].value);
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
	loud = eeprom_read_byte(eepromLoudness);
	chanCnt = eeprom_read_byte(eepromChanCnt);
	tdaIC = eeprom_read_byte(eepromICSelect);

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
	eeprom_write_byte(eepromChannel, chan);
	eeprom_write_byte(eepromLoudness, loud);
	eeprom_write_byte(eepromChanCnt, chanCnt);
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
	if (chan >= chanCnt)
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

	double mult = 1;

	if (param->label == volumeLabel
	 || param->label == preampLabel
	 || param->label == balanceLabel)
	{
		mult = 1.25;
	}
	if (param->label == gainLabel0
	 || param->label == gainLabel1
	 || param->label == gainLabel2
	 || param->label == gainLabel3)
	{
		mult = 1.875;
	}
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
	showParValue(param->value * param->step * mult);
	showParLabel(param->label);
}
