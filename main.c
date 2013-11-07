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
	.label = "Громкость",
	.i2cValue = &i2cVolume,
	.dbValue = &dbVolume,
};

regParam bass = {
	.min = -7,
	.max = 7,
	.label = " Тембр НЧ",
	.i2cValue = &i2cFreq,
	.dbValue = &dbFreq,
};

regParam middle = {
	.min = -7,
	.max = 7,
	.label = " Тембр СЧ",
	.i2cValue = &i2cFreq,
	.dbValue = &dbFreq,
};

regParam treble = {
	.min = -7,
	.max = 7,
	.label = " Тембр ВЧ",
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
				marker = 1000;
			}
			changeParam(curParam, delta);

			gdSetPos(0, 0);
			gdWriteString(curParam->label);
			gdWriteNum(curParam->dbValue(curParam->rawValue), 4);
			gdWriteString(db);

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
