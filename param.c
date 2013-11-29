#include "param.h"

#include <avr/eeprom.h>
#include "ks0108.h"

void loadParams(void)
{
	volume = eeprom_read_byte((void*)0);
	bass = eeprom_read_byte((void*)1);
	middle = eeprom_read_byte((void*)2);
	treble = eeprom_read_byte((void*)3);
	balance = eeprom_read_byte((void*)4);
}

void saveParams(void)
{
	eeprom_write_byte((void*)0, volume);
	eeprom_write_byte((void*)1, bass);
	eeprom_write_byte((void*)2, middle);
	eeprom_write_byte((void*)3, treble);
	eeprom_write_byte((void*)4, balance);
}

void incVolume(void) { volume++; if (volume > VOL_MAX) volume = VOL_MAX; }
void decVolume(void) { volume--; if (volume < VOL_MIN) volume = VOL_MIN; }

void incBMT(int8_t *par) { *par += 2; if (*par > BMT_MAX) *par = BMT_MAX; }
void decBMT(int8_t *par) { *par -= 2; if (*par < BMT_MIN) *par = BMT_MIN; }

void incBalance(void) { balance++; if (balance > BAL_MAX) balance = BAL_MAX; }
void decBalance(void) { balance--; if (balance < BAL_MIN) balance = BAL_MIN; }

uint8_t db[] = "дБ";

void showParLabel(int8_t *par, uint8_t *parLabel)
{
	gdWriteString2(0, 8, parLabel);
	gdSetPos(100, 40);
	gdWriteNum(volume, 4);
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
