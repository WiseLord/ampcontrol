#include "param.h"

#include <avr/eeprom.h>
#include <util/delay.h>

#include "ks0108.h"
#include "i2c.h"

void setVolume(int8_t val)
{
	I2CWrite(0b10001000, FUNC_VOLUME, -val);
}

void setSpeaker(int8_t val)
{
	int8_t spLeft = val;
	int8_t spRight = val;

	if (balance > 0) {
		spLeft -= balance;
		if (spLeft < SPK_MIN)
			spLeft = SPK_MIN;
	} else {
		spRight += balance;
		if (spRight < SPK_MIN)
			spRight = SPK_MIN;
	}

	I2CWrite(0b10001000, FUNC_SPEAKER_LEFT, -spLeft);
	I2CWrite(0b10001000, FUNC_SPEAKER_RIGHT, -spRight);
}

void setBMT(int8_t address, int8_t val)
{
	val = val >> 1;
	if (val > 0)
		val = 15 - val;
	else
		val = 7 + val;
	I2CWrite(0b10001000, address, val);
}

void setChannel(uint8_t ch)
{
	channel = ch;
	I2CWrite(0b10001000, FUNC_INPUT_SELECT, 3 - ch);
}

void setGain(uint8_t ch, int8_t val)
{
	val = val >> 1;
	I2CWrite(0b10001000, FUNC_INPUT_GAIN, val);
}

void loadParams(void)
{
	volume = eeprom_read_byte((void*)0);
	bass = eeprom_read_byte((void*)1);
	middle = eeprom_read_byte((void*)2);
	treble = eeprom_read_byte((void*)3);
	balance = eeprom_read_byte((void*)4);
	speaker = eeprom_read_byte((void*)5);
	channel = eeprom_read_byte((void*)6);
	gain[0] = eeprom_read_byte((void*)7);
	gain[1] = eeprom_read_byte((void*)8);
	gain[2] = eeprom_read_byte((void*)9);
	gain[3] = eeprom_read_byte((void*)10);
	spMode = eeprom_read_byte((void*)11);

	I2CWrite(0b10001000, FUNC_INPUT_GAIN, 0);
	setVolume(volume);
	setChannel(channel);
	I2CWrite(0b10001000, FUNC_SPEAKER_LEFT, 0);
	I2CWrite(0b10001000, FUNC_SPEAKER_RIGHT, 0);
	int8_t i;
	for (i = SPK_MIN; i <= speaker; i++) {
		setSpeaker(i);
		_delay_ms(20);
	}
}

void saveParams(void)
{
	eeprom_write_byte((void*)0, volume);
	eeprom_write_byte((void*)1, bass);
	eeprom_write_byte((void*)2, middle);
	eeprom_write_byte((void*)3, treble);
	eeprom_write_byte((void*)4, balance);
	eeprom_write_byte((void*)5, speaker);
	eeprom_write_byte((void*)6, channel);
	eeprom_write_byte((void*)7, gain[0]);
	eeprom_write_byte((void*)8, gain[1]);
	eeprom_write_byte((void*)9, gain[2]);
	eeprom_write_byte((void*)10, gain[3]);
	eeprom_write_byte((void*)11, spMode);
}

void editSpMode()
{
	if (spMode == MODE_STEREO)
		spMode = MODE_MIXED;
	else
		spMode = MODE_STEREO;
}

void incVolume(void)
{
	volume++;
	if (volume > VOL_MAX)
		volume = VOL_MAX;
	setVolume(volume);
}

void decVolume(void)
{
	volume--;
	if (volume < VOL_MIN)
		volume = VOL_MIN;
	setVolume(volume);
}

void incSpeaker(void)
{
	speaker++;
	if (speaker > SPK_MAX)
		speaker = SPK_MAX;
	setSpeaker(speaker);
}

void decSpeaker(void)
{
	speaker--;
	if (speaker < SPK_MIN)
		speaker = SPK_MIN;
	setSpeaker(speaker);
}

void incBMT(int8_t *par)
{
	*par += 2;
	if (*par > BMT_MAX)
		*par = BMT_MAX;
	if (par == &bass)
		setBMT(FUNC_BASS, *par);
	else if (par == &middle)
		setBMT(FUNC_MIDDLE, *par);
	else if (par == &treble)
		setBMT(FUNC_TREBLE, *par);
}

void decBMT(int8_t *par)
{
	*par -= 2;
	if (*par < BMT_MIN)
		*par = BMT_MIN;
	if (par == &bass)
		setBMT(FUNC_BASS, *par);
	else if (par == &middle)
		setBMT(FUNC_MIDDLE, *par);
	else if (par == &treble)
		setBMT(FUNC_TREBLE, *par);
}

void incChannel(void)
{
	channel++;
	if (channel > 3)
		channel = 0;
	setChannel(channel);
}

void incGain(uint8_t chan)
{
	gain[chan] += 2;
	if (gain[chan] > GAIN_MAX)
			gain[chan] = GAIN_MAX;
	setGain(chan, gain[chan]);
}

void decGain(uint8_t chan)
{
	gain[chan] -= 2;
	if (gain[chan] < GAIN_MIN)
			gain[chan] = GAIN_MIN;
	setGain(chan, gain[chan]);
}

void incBalance(void) {
	balance++;
	if (balance > BAL_MAX)
		balance = BAL_MAX;
	setSpeaker(speaker);
}

void decBalance(void) {
	balance--;
	if (balance < BAL_MIN)
		balance = BAL_MIN;
	setSpeaker(speaker);
}

uint8_t db[] = "дБ";

void showParLabel(int8_t *par, const uint8_t *parLabel)
{
	gdSetXY(0, 0);
	gdWriteStringScaledProgmem(parLabel, 2, 3, 0);
	gdSetXY(113, 0);
	gdWriteStringScaled(mkNumString(channel + 1, 1, ' '), 3, 3, 0);
	gdSetXY(94, 4);
	gdWriteStringScaled(mkNumString(*par, 3, ' '), 2, 2, 0);
	gdSetXY(106, 6);
	gdWriteStringScaled(db, 2, 2, 0);
}

void showBar(uint8_t length, int8_t from, int8_t to)
{
	int8_t i, j;
	uint8_t data;
	for (j = 5; j <=6; j++) {
		gdWriteCommand(KS0108_SET_ADDRESS, CS1 | CS2);
		gdWriteCommand(KS0108_SET_PAGE + j, CS1 | CS2);
		for (i = 0; i < length; i++) {
			if (j == 5)
				data = 0x80;
			else
				data = 0x01;
			if (i >= from && i <= to)
				data = 0xFF;
			if (i % 2)
				data = 0x00;
			gdWriteData(data, i < 64 ? CS1 : CS2);
		}
	}
}

void showVolume(const uint8_t *parLabel)
{
	int8_t r;
	r = 2 * volume + 93;
	showBar(94, 0, r);
	showParLabel(&volume, parLabel);
}

void showBMT(int8_t *par, const uint8_t *parLabel)
{
	int8_t l = 42, r = 42;
	if (*par > 0)
		r += *par * 3;
	else
		l += *par * 3;
	showParLabel(par, parLabel);
	showBar(86, l, r);
}

void showBalance(const uint8_t *parLabel)
{
	int8_t l = 42, r = 42;
	if (balance > 0)
		r += balance * 2;
	else
		l += balance * 2;
	showParLabel(&balance, parLabel);
	showBar(86, l, r);
}

void showSpeaker(const uint8_t *parLabel)
{
	int8_t r;
	r = speaker + 78;
	showBar(79, 0, r);
	showParLabel(&speaker, parLabel);
}

void showGain(uint8_t chan, const uint8_t *parLabel)
{
	int8_t r;
	r = 3 * gain[chan] - 1;
	showBar(90, 0, r);
	showParLabel(gain + chan, parLabel);
}
