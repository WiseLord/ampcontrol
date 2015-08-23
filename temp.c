#include "temp.h"

#include <avr/eeprom.h>
#include "eeprom.h"
#include "input.h"

static int8_t tempTH;

void loadTempParams(void)
{
	tempTH = eeprom_read_byte((uint8_t*)EEPROM_TEMP_TH);

	return;
}

void saveTempParams(void)
{
	eeprom_update_byte((uint8_t*)EEPROM_TEMP_TH, tempTH);

	return;
}

void tempInit(void)
{
	loadTempParams();

	DDR(FAN1) |= FAN1_WIRE;
	DDR(FAN2) |= FAN2_WIRE;

	PORT(FAN1) &= ~FAN1_WIRE;
	PORT(FAN2) &= ~FAN2_WIRE;

	return;
}

void tempControlProcess(void)
{
	int8_t temp1, temp2;

	temp1 = ds18x20GetTemp(0) / 10;
	temp2 = ds18x20GetTemp(1) / 10;

	if (temp1 >= tempTH)
		PORT(FAN1) |= FAN1_WIRE;
	else if (temp1 <= tempTH - 5)
		PORT(FAN1) &= ~FAN1_WIRE;

	if (temp2 >= tempTH)
		PORT(FAN2) |= FAN2_WIRE;
	else if (temp2 <= tempTH - 5)
		PORT(FAN2) &= ~FAN2_WIRE;

	return;
}

void setTempTH(int8_t temp)
{
	temp = temp;

	return;
}

int8_t getTempTH(void)
{
	return tempTH;
}

void changeTempTH(int8_t diff)
{
	tempTH += diff;
	if (tempTH > MAX_TEMP)
		tempTH = MAX_TEMP;
	if (tempTH < MIN_TEMP)
		tempTH = MIN_TEMP;

	return;
}

