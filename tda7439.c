#include <avr/eeprom.h>

#include "tda7439.h"
#include "ks0108.h"

uint8_t db[] = "дБ";

static int8_t i2cFreq(int8_t value)
{
	return value > 0 ? 15 - value : 7 + value;
}

static int8_t dbFreq(int8_t value)
{
	return value << 1;
}

static int8_t i2cVolume(int8_t value)
{
	return -value;
}

static int8_t dbVolume(int8_t value)
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

void changeParam(regParam *param, int8_t encVal)
{
	param->value += encVal;
	if (param->value > param->max)
		param->value = param->max;
	if (param->value < param->min)
		param->value = param->min;
}

void incParam(regParam *param)
{
	if (param->value < param->max)
		param->value++;
}

void decParam(regParam *param)
{
	if (param->value > param->min)
		param->value--;
}

void showParam(regParam *param)
{
	gdWriteString2(0, 8, param->label);

	uint8_t i, j;
	uint16_t l;
	uint8_t data;
	l = 96 * (1 + param->value - param->min) / (1 + param->max - param->min);
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
	gdWriteNum(param->dbValue(param->value), 4);
	gdSetPos(112, 48);
	gdWriteString(db);
}

regParam *nextParam(regParam * param)
{
	if (param == &volume)
		return &bass;
	if (param == &bass)
		return &middle;
	if (param == &middle)
		return &treble;
	return &volume;
}

void loadParams(void)
{
	volume.value = eeprom_read_byte((void*)0);
	bass.value = eeprom_read_byte((void*)1);
	middle.value = eeprom_read_byte((void*)2);
	treble.value = eeprom_read_byte((void*)3);
}

void saveParams(void)
{
	eeprom_write_byte((void*)0, volume.value);
	eeprom_write_byte((void*)1, bass.value);
	eeprom_write_byte((void*)2, middle.value);
	eeprom_write_byte((void*)3, treble.value);
}
