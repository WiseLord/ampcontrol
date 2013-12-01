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

	I2CWrite(0b10001000, FUNC_INPUT_GAIN, 0);
	setVolume(volume);
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

void showParLabel(int8_t *par, uint8_t *parLabel)
{
	gdSetPos(122, 0);
	gdWriteNum(channel + 1, 1);
	gdWriteString2(0, 8, parLabel);
	gdSetPos(100, 40);
	gdWriteNum(*par, 4);
	gdSetPos(112, 48);
	gdWriteString(db);
}

void showVolume(uint8_t *parLabel)
{
	showParLabel(&volume, parLabel);

	uint8_t i, j;
	int16_t r;
	uint8_t data;
	r = 2 * volume + 94;
	for (j = 5; j <=6; j++) {
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_PAGE + j, CS1 | CS2);
		for (i = 0; i < 94; i++) {
			if (j == 5)
				data = 0x80;
			else
				data = 0x01;
			if (i < r)
				data = 0xFF;
			if (i % 2)
				data = 0x00;
			gdWrite(GD_DATA, data, i < 64 ? CS1 : CS2);
		}
	}
}

void showBMT(int8_t *par, uint8_t *parLabel)
{
	showParLabel(par, parLabel);

	uint8_t i, j;
	uint16_t l, r;
	uint8_t data;

	if (*par > 0) {
		l = 42;
		r = 42 + *par * 3;
	} else {
		l = 42 + *par * 3;
		r = 42;
	}

	for (j = 5; j <=6; j++) {
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_PAGE + j, CS1 | CS2);
		for (i = 0; i < 86; i++) {
			if (j == 5)
				data = 0x80;
			else
				data = 0x01;
			if (i >= l && i <= r)
				data = 0xFF;
			if (i % 2)
				data = 0x00;
			gdWrite(GD_DATA, data, i < 64 ? CS1 : CS2);
		}
	}
}

void showBalance(uint8_t *parLabel)
{
	showParLabel(&balance, parLabel);

	uint8_t i, j;
	uint16_t l, r;
	uint8_t data;

	if (balance > 0) {
		l = 42;
		r = 42 + balance * 2;
	} else {
		l = 42 + balance * 2;
		r = 42;
	}

	for (j = 5; j <=6; j++) {
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_PAGE + j, CS1 | CS2);
		for (i = 0; i < 86; i++) {
			if (j == 5)
				data = 0x80;
			else
				data = 0x01;
			if (i >= l && i <= r)
				data = 0xFF;
			if (i % 2)
				data = 0x00;
			gdWrite(GD_DATA, data, i < 64 ? CS1 : CS2);
		}
	}
}

void showSpeaker(uint8_t *parLabel)
{
	showParLabel(&speaker, parLabel);

	uint8_t i, j;
	int16_t r;
	uint8_t data;
	r = speaker + 79;
	for (j = 5; j <=6; j++) {
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_PAGE + j, CS1 | CS2);
		for (i = 0; i < 79; i++) {
			if (j == 5)
				data = 0x80;
			else
				data = 0x01;
			if (i < r)
				data = 0xFF;
			if (i % 2)
				data = 0x00;
			gdWrite(GD_DATA, data, i < 64 ? CS1 : CS2);
		}
	}
}

void showGain(uint8_t chan, uint8_t *parLabel)
{
	showParLabel(gain + chan, parLabel);

	uint8_t i, j;
	int16_t r;
	uint8_t data;
	r = 3 * gain[chan];
	for (j = 5; j <=6; j++) {
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_PAGE + j, CS1 | CS2);
		for (i = 0; i < 90; i++) {
			if (j == 5)
				data = 0x80;
			else
				data = 0x01;
			if (i < r)
				data = 0xFF;
			if (i % 2)
				data = 0x00;
			gdWrite(GD_DATA, data, i < 64 ? CS1 : CS2);
		}
	}
}
