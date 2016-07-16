#include "alarm.h"

#include "i2c.h"
#include "audio/audioproc.h"

#define DS1307_ADDR			0b11010000
#define NOEDIT				0xFF

static int8_t alarm[4];

static uint8_t _eam = NOEDIT;	/* Edit alarm mode */

int8_t getAlarm(uint8_t am)
{
	return alarm[am - RTC_A0_HOUR];
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
	I2CWriteByte(RTC_A0_HOUR);
	I2CStart(DS1307_ADDR | I2C_READ);
	for (i = RTC_A0_HOUR; i < RTC_A0_WDAY; i++) {
		temp = I2CReadByte(I2C_ACK);
		alarm[i - RTC_A0_HOUR] = temp;
	}
	temp = I2CReadByte(I2C_NOACK);
	alarm[RTC_A0_WDAY - RTC_A0_HOUR] = temp;
	I2CStop();

	return alarm;
}

static void writeAlarm(void)
{
	uint8_t i;

	I2CStart(DS1307_ADDR);
	I2CWriteByte(RTC_A0_HOUR);
	for (i = RTC_A0_HOUR; i <= RTC_A0_WDAY; i++)
		I2CWriteByte(alarm[i - RTC_A0_HOUR]);
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
		_eam = RTC_A0_HOUR;
		break;
	case RTC_A0_HOUR:
		_eam = RTC_A0_MIN;
		break;
	case RTC_A0_MIN:
		_eam = RTC_A0_INPUT;
		break;
	case RTC_A0_INPUT:
		_eam = RTC_A0_WDAY;
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
	case RTC_A0_HOUR:
		alarm[RTC_A0_HOUR - RTC_A0_HOUR] += diff;
		if (alarm[RTC_A0_HOUR - RTC_A0_HOUR] > 23)
			alarm[RTC_A0_HOUR - RTC_A0_HOUR] = 0;
		if (alarm[RTC_A0_HOUR - RTC_A0_HOUR] < 0)
			alarm[RTC_A0_HOUR - RTC_A0_HOUR] = 23;
		break;
	case RTC_A0_MIN:
		alarm[RTC_A0_MIN - RTC_A0_HOUR] += diff;
		if (alarm[RTC_A0_MIN - RTC_A0_HOUR] > 59)
			alarm[RTC_A0_MIN - RTC_A0_HOUR] = 0;
		if (alarm[RTC_A0_MIN - RTC_A0_HOUR] < 0)
			alarm[RTC_A0_MIN - RTC_A0_HOUR] = 59;
		break;
	case RTC_A0_INPUT:
		alarm[RTC_A0_INPUT - RTC_A0_HOUR] += diff;
		if (alarm[RTC_A0_INPUT - RTC_A0_HOUR] >= sndInputCnt())
			alarm[RTC_A0_INPUT - RTC_A0_HOUR] = 0;
		if (alarm[RTC_A0_INPUT - RTC_A0_HOUR] < 0)
			alarm[RTC_A0_INPUT - RTC_A0_HOUR] = sndInputCnt() - 1;
		break;
	case RTC_A0_WDAY:
		alarm[RTC_A0_WDAY - RTC_A0_HOUR] += diff;
		if (alarm[RTC_A0_WDAY - RTC_A0_HOUR] < -64)
			alarm[RTC_A0_WDAY - RTC_A0_HOUR] = 0;
		if (alarm[RTC_A0_WDAY - RTC_A0_HOUR] < 0)
			alarm[RTC_A0_WDAY - RTC_A0_HOUR] = 127;
		break;
	default:
		break;
	}
	writeAlarm();

	return;
}
