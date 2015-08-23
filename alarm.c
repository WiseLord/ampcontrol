#include "alarm.h"

#include "i2c.h"
#include "audio/audioproc.h"

#define DS1307_ADDR			0b11010000
#define NOEDIT				0xFF

static int8_t alarm[4];

static uint8_t _eam = NOEDIT;	/* Edit alarm mode */

int8_t getAlarm(uint8_t am)
{
	return alarm[am - DS1307_A0_HOUR];
}

uint8_t getEam(void)
{
	return _eam;
}

int8_t *readAlarm(void)
{
	uint8_t temp;
	uint8_t i;

	I2CStart(DS1307_ADDR);
	I2CWriteByte(DS1307_A0_HOUR);
	I2CStart(DS1307_ADDR | I2C_READ);
	for (i = DS1307_A0_HOUR; i < DS1307_A0_WDAY; i++) {
		temp = I2CReadByte(I2C_ACK);
		alarm[i - DS1307_A0_HOUR] = temp;
	}
	temp = I2CReadByte(I2C_NOACK);
	alarm[DS1307_A0_WDAY - DS1307_A0_HOUR] = temp;
	I2CStop();

	return alarm;
}

static void writeAlarm(void)
{
	uint8_t i;

	I2CStart(DS1307_ADDR);
	I2CWriteByte(DS1307_A0_HOUR);
	for (i = DS1307_A0_HOUR; i <= DS1307_A0_WDAY; i++)
		I2CWriteByte(alarm[i - DS1307_A0_HOUR]);
	I2CStop();

	return;
}

void stopEditAlarm(void)
{
	_eam = NOEDIT;

	return;
}

uint8_t isEAM(void)
{
	if (_eam == NOEDIT)
		return 0;
	return 1;
}

void editAlarm(void)
{
	switch (_eam) {
	case NOEDIT:
		_eam = DS1307_A0_HOUR;
		break;
	case DS1307_A0_HOUR:
		_eam = DS1307_A0_MIN;
		break;
	case DS1307_A0_MIN:
		_eam = DS1307_A0_INPUT;
		break;
	case DS1307_A0_INPUT:
		_eam = DS1307_A0_WDAY;
		break;
	default:
		_eam = NOEDIT;
		break;
	}

	return;
}

void changeAlarm(int diff)
{
	switch (_eam) {
	case DS1307_A0_HOUR:
		alarm[DS1307_A0_HOUR - DS1307_A0_HOUR] += diff;
		if (alarm[DS1307_A0_HOUR - DS1307_A0_HOUR] > 23)
			alarm[DS1307_A0_HOUR - DS1307_A0_HOUR] = 0;
		if (alarm[DS1307_A0_HOUR - DS1307_A0_HOUR] < 0)
			alarm[DS1307_A0_HOUR - DS1307_A0_HOUR] = 23;
		break;
	case DS1307_A0_MIN:
		alarm[DS1307_A0_MIN - DS1307_A0_HOUR] += diff;
		if (alarm[DS1307_A0_MIN - DS1307_A0_HOUR] > 59)
			alarm[DS1307_A0_MIN - DS1307_A0_HOUR] = 0;
		if (alarm[DS1307_A0_MIN - DS1307_A0_HOUR] < 0)
			alarm[DS1307_A0_MIN - DS1307_A0_HOUR] = 59;
		break;
	case DS1307_A0_INPUT:
		alarm[DS1307_A0_INPUT - DS1307_A0_HOUR] += diff;
		if (alarm[DS1307_A0_INPUT - DS1307_A0_HOUR] >= sndInputCnt())
			alarm[DS1307_A0_INPUT - DS1307_A0_HOUR] = 0;
		if (alarm[DS1307_A0_INPUT - DS1307_A0_HOUR] < 0)
			alarm[DS1307_A0_INPUT - DS1307_A0_HOUR] = sndInputCnt() - 1;
		break;
	case DS1307_A0_WDAY:
		alarm[DS1307_A0_WDAY - DS1307_A0_HOUR] += diff;
		if (alarm[DS1307_A0_WDAY - DS1307_A0_HOUR] < -64)
			alarm[DS1307_A0_WDAY - DS1307_A0_HOUR] = 0;
		if (alarm[DS1307_A0_WDAY - DS1307_A0_HOUR] < 0)
			alarm[DS1307_A0_WDAY - DS1307_A0_HOUR] = 127;
		break;
	default:
		break;
	}
	writeAlarm();

	return;
}
