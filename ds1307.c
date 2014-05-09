#include "ds1307.h"

#include <avr/pgmspace.h>

#include "i2c.h"

#include "ks0108.h"
#include "eeprom.h"

static int8_t time[7];
static timeMode etm = NOEDIT;

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
		if (time[YEAR] & 0x03) {
			return 28;
		} else {
			return 29;
		}
	}

	if (time[MONTH] == 4 || time[MONTH] == 6 || time[MONTH] == 9 || time[MONTH] == 11) {
		return 30;
	}

	return 31;
}

static int8_t *getTime(void)
{
	uint8_t temp;
	uint8_t i;

	for (i = SEC; i <= YEAR; i++) {
		I2CRead(DS1307_ADDR, i, &temp);
		time[i] = BD2D(temp);
	}

	return time;
}

static void setTime(void)
{
	uint8_t i;

	if (time[DAY] > daysInMonth())
		time[DAY] = daysInMonth();
	if (etm >= DAY)
		calcWeekDay();

	for (i = SEC; i <= YEAR; i++) {
		I2CWrite(DS1307_ADDR, i, D2BD(time[i]));
	}

	return;
}

static void drawTm(timeMode tm, const uint8_t *font)
{
	if (etm == tm)
		gdLoadFont(font, 0);
	else
		gdLoadFont(font, 1);
	gdWriteString(mkNumString(time[tm], 2, '0', 10));
	gdLoadFont(font, 1);
}

void showTime()
{
	getTime();
	gdSetXY(4, 0);

	drawTm(HOUR, font_digits_32);
	gdWriteString((uint8_t*)"\x7F:\x7F");
	drawTm(MIN, font_digits_32);
	gdWriteString((uint8_t*)"\x7F:\x7F");
	drawTm(SEC, font_digits_32);

	gdSetXY(9, 4);

	drawTm(DAY, font_ks0066_ru_24);
	gdWriteString((uint8_t*)"\x7F.\x7F");
	drawTm(MONTH, font_ks0066_ru_24);
	gdWriteString((uint8_t*)"\x7F.\x7F");
	if (etm == YEAR)
		gdLoadFont(font_ks0066_ru_24, 0);
	gdWriteString((uint8_t*)"20");
	drawTm(YEAR, font_ks0066_ru_24);

	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(32, 7);
	gdWriteStringEeprom(mondayLabel + 16 * (time[WEEK] % 7));

	return;
}

void stopEditTime(void)
{
	etm = NOEDIT;

	return;
}

uint8_t isETM(void)
{
	if (etm == NOEDIT)
		return 0;
	return 1;
}

void editTime(void)
{
	switch (etm) {
	case NOEDIT:
		etm = HOUR;
		break;
	case HOUR:
	case MIN:
		etm--;
		break;
	case SEC:
		etm = DAY;
		break;
	case DAY:
	case MONTH:
		etm++;
		break;
	default:
		etm = NOEDIT;
		break;
	}
	showTime(0);
}

void changeTime(int diff)
{
	getTime();
	switch (etm) {
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
	setTime();
}
