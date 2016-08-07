#include "alarm.h"

#include "i2c.h"
#include "audio/audio.h"
#include <avr/eeprom.h>
#include "eeprom.h"

ALARM_type alarm0;

void alarmInit(void)
{
	eeprom_read_block(&alarm0, (void*)EEPROM_A0_HOUR, sizeof(ALARM_type) - 1);

	return;
}

void alarmSave(void)
{
	eeprom_update_block(&alarm0, (void*)EEPROM_A0_HOUR, sizeof(ALARM_type) - 1);
	alarm0.eam = ALARM_NOEDIT;

	return;
}

void alarmNextEditParam(void)
{
	switch (alarm0.eam) {
	case ALARM_HOUR:
	case ALARM_MIN:
	case ALARM_INPUT:
		alarm0.eam++;
		break;
	default:
		alarm0.eam = ALARM_HOUR;
		break;
	}

	return;
}

void alarmChangeTime(int diff)
{
	switch (alarm0.eam) {
	case ALARM_HOUR:
		alarm0.hour += diff;
		if (alarm0.hour > 23)
			alarm0.hour = 0;
		if (alarm0.hour < 0)
			alarm0.hour = 23;
		break;
	case ALARM_MIN:
		alarm0.min += diff;
		if (alarm0.min > 59)
			alarm0.min = 0;
		if (alarm0.min < 0)
			alarm0.min = 59;
		break;
	case ALARM_INPUT:
		alarm0.input += diff;
		if (alarm0.input >= aproc.inCnt)
			alarm0.input = 0;
		if (alarm0.input < 0)
			alarm0.input = aproc.inCnt - 1;
		break;
	case ALARM_WDAY:
		alarm0.wday += diff;
		if (alarm0.wday < -64)
			alarm0.wday = 0;
		if (alarm0.wday < 0)
			alarm0.wday = 127;
		break;
	default:
		break;
	}

	return;
}
