#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"

typedef struct {
	int8_t rawValue;
	int8_t min;
	int8_t max;
	uint8_t label[16];
	int8_t (*i2cValue)(int8_t value);
	int8_t (*dbValue)(int8_t value);
} regParam;

uint8_t db[] = "дБ";

int8_t i2cFreq(int8_t value)
{
	return value > 0 ? 15 - value : 7 + value;
}

int8_t dbFreq(int8_t value)
{
	return value << 1;
}

int8_t i2cVolume(int8_t value)
{
	return -value;
}

int8_t dbVolume(int8_t value)
{
	return value;
}

regParam volume = {
	.min = -47,
	.max = 0,
	.label = " Громкость",
	.i2cValue = &i2cVolume,
	.dbValue = &dbVolume,
};

regParam bass = {
	.min = -7,
	.max = 7,
	.label = " Тембр НЧ ",
	.i2cValue = &i2cFreq,
	.dbValue = &dbFreq,
};

regParam middle = {
	.min = -7,
	.max = 7,
	.label = " Тембр СЧ ",
	.i2cValue = &i2cFreq,
	.dbValue = &dbFreq,
};

regParam treble = {
	.min = -7,
	.max = 7,
	.label = " Тембр ВЧ ",
	.i2cValue = &i2cFreq,
	.dbValue = &dbFreq,
};

regParam *curParam;

uint8_t getButtons() {
	return (~BTN_PIN & BTN_MASK);
}

void changeParam(regParam *param, int8_t encVal)
{
	param->rawValue += encVal;
	if (param->rawValue > param->max)
		param->rawValue = param->max;
	if (param->rawValue < param->min)
		param->rawValue = param->min;
}

void changeCurParam()
{
	if (curParam == &volume)
		curParam = &bass;
	else if (curParam == &bass)
		curParam = &middle;
	else if (curParam == &middle)
		curParam = &treble;
	else if (curParam == &treble)
		curParam = &volume;
}

void showParam(regParam *param)
{
	gdWriteString2(0, 8, param->label);

	uint8_t i, j;
	uint16_t l;
	uint8_t data;
	l = 96 * (1 + param->rawValue - param->min) / (1 + param->max - param->min);
	for (j = 5; j <=6; j++)
	{
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_PAGE + j, CS1 | CS2);
		for (i = 0; i < 96; i++)
		{
			data = 0x00;
			if ((i < l) && (i % 2 == 0))
				data = 0xFF;
			gdWrite(GD_DATA, data, i < 64 ? CS1 : CS2);
		}
	}
	gdSetPos(100, 40);
	gdWriteNum(param->dbValue(param->rawValue), 4);
	gdSetPos(112, 48);
	gdWriteString(db);
}

int main(void)
{
	_delay_ms(100);
	gdInit();
	adcInit();
	btnInit();

	sei();

	int8_t delta;
	uint8_t btn, prevBtn;
	uint8_t *buf;
	int16_t marker = 0;

	volume.rawValue = eeprom_read_byte((void*)0);
	bass.rawValue = eeprom_read_byte((void*)1);
	middle.rawValue = eeprom_read_byte((void*)2);
	treble.rawValue = eeprom_read_byte((void*)3);

	curParam = &volume;
	prevBtn = 0;

	while (1)
	{
		btn = getButtons();
		if ((btn == BTN_MENU) && (btn != prevBtn))
		{
			if (marker)
				changeCurParam();
		}
		prevBtn = btn;

		delta = getEncValue();
		if ((btn == BTN_MENU) | delta | marker)
		{
			if (btn | delta)
			{
				if (!marker)
					gdFill(0x00, CS1 | CS2);
				marker = 200;
			}
			changeParam(curParam, delta);

			showParam(curParam);

			marker--;
			if (marker == 0)
			{
				eeprom_write_byte((void*)0, volume.rawValue);
				eeprom_write_byte((void*)1, bass.rawValue);
				eeprom_write_byte((void*)2, middle.rawValue);
				eeprom_write_byte((void*)3, treble.rawValue);
				curParam = &volume;
			}
		}
		else
		{
			buf = getData();
			gdSpectrum(buf, MODE_STEREO);
		}
	}
	gdFill(0x00, CS1 | CS2);
	return 0;
}
