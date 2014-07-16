#include "ds1307.h"

#include "i2c.h"

static int8_t time[7];
static timeMode _etm = NOEDIT;

int8_t getTime(timeMode tm)
{
	return time[tm];
}

timeMode getEtm()
{
	return _etm;
}

static void calcWeekDay(void)
{
	uint8_t a, y, m;

	a = (time[MONTH] > 2 ? 0 : 1);
	y = 12 + time[YEAR] - a;
	m = time[MONTH] + 12 * a - 2;

	time[WEEK] = (time[DAY] + y + (y / 4) - 1 + ((31 * m) / 12)) % 7;
	if (time[WEEK] == 0)
		time[WEEK] = 7;

	return;
}

static uint8_t daysInMonth()
{
	if (time[MONTH] == 2) {
		if (time[YEAR] & 0x03)
			return 28;
		return 29;
	}

	if (time[MONTH] == 4 || time[MONTH] == 6 || time[MONTH] == 9 || time[MONTH] == 11) {
		return 30;
	}

	return 31;
}

int8_t *readTime(void)
{
	uint8_t temp;
	uint8_t i;

	for (i = SEC; i <= YEAR; i++) {
		I2CStart(DS1307_ADDR);
		I2CWriteByte(i);
		I2CStart(DS1307_ADDR | I2C_READ);
		I2CReadByte(&temp, I2C_NOACK);
		I2CStop();
		time[i] = BD2D(temp);
	}

	return time;
}

static void writeTime(void)
{
	uint8_t i;

	if (time[DAY] > daysInMonth())
		time[DAY] = daysInMonth();
	if (_etm >= DAY)
		calcWeekDay();

	for (i = SEC; i <= YEAR; i++) {
		I2CStart(DS1307_ADDR);
		I2CWriteByte(i);
		I2CWriteByte(D2BD(time[i]));
		I2CStop();
	}

	return;
}

void stopEditTime(void)
{
	_etm = NOEDIT;

	return;
}

uint8_t isETM(void)
{
	if (_etm == NOEDIT)
		return 0;
	return 1;
}

void editTime(void)
{
	switch (_etm) {
	case NOEDIT:
		_etm = HOUR;
		break;
	case HOUR:
	case MIN:
		_etm--;
		break;
	case SEC:
		_etm = DAY;
		break;
	case DAY:
	case MONTH:
		_etm++;
		break;
	default:
		_etm = NOEDIT;
		break;
	}
}

void changeTime(int diff)
{
	readTime();
	switch (_etm) {
	case HOUR:
		time[HOUR] += diff;
		if (time[HOUR] > 23)
			time[HOUR] = 0;
		if (time[HOUR] < 0)
			time[HOUR] = 23;
		break;
	case MIN:
		time[MIN] += diff;
		if (time[MIN] > 59)
			time[MIN] = 0;
		if (time[MIN] < 0)
			time[MIN] = 59;
		break;
	case SEC:
		time[SEC] = 0;
		break;
	case DAY:
		time[DAY] += diff;
		if (time[DAY] > daysInMonth())
			time[DAY] = 1;
		if (time[DAY] < 1)
			time[DAY] = daysInMonth();
		break;
	case MONTH:
		time[MONTH] += diff;
		if (time[MONTH] > 12)
			time[MONTH] = 1;
		if (time[MONTH] < 1)
			time[MONTH] = 12;
		break;
	case YEAR:
		time[YEAR] += diff;
		if (time[YEAR] > 99)
			time[YEAR] = 0;
		if (time[YEAR] < 0)
			time[YEAR] = 99;
		break;
	default:
		break;
	}
	writeTime();
}
