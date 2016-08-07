#include "alarm.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "audio/audio.h"
#include "eeprom.h"

ALARM_type alarm0;

const static ALARM_type alarmMin PROGMEM = {0, 0, 0, 0, ALARM_NOEDIT};
const static ALARM_type alarmMax PROGMEM = {23, 59, 6, 0x7F, ALARM_NOEDIT};

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
	int8_t *alarm = (int8_t*)&alarm0 + alarm0.eam;
	int8_t aMax = pgm_read_byte((int8_t*)&alarmMax + alarm0.eam);
	int8_t aMin = pgm_read_byte((int8_t*)&alarmMin + alarm0.eam);

	if (alarm0.eam == ALARM_INPUT)
		aMax = aproc.inCnt - 1;

	*alarm += diff;
	if (alarm0.eam == ALARM_WDAY)
		*alarm &= 0x7F;

	if (*alarm > aMax)
		*alarm = aMin;
	if (*alarm < aMin)
		*alarm = aMax;

	return;
}
