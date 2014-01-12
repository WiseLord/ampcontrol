#include "ds1307.h"

#include <avr/pgmspace.h>

#include "i2c.h"
#include "ks0108.h"
#include "input.h"
#include "eeprom.h"

int8_t hour, minute, second, day, month, year, weekday;

uint8_t bd2d(uint8_t temp)
{
	return (temp >> 4) * 10 + (temp & 0x0F);
}

uint8_t d2bd(uint8_t temp)
{
	return ((temp / 10) << 4) + (temp % 10);
}

void calcWeekDay(void)
{
	int16_t a = (14 - month) / 12;
	int16_t y = 2000 + year - a;
	int16_t m = month + 12 * a - 2;
	weekday = (7000 + (day + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12)) % 7;
	if (weekday == 0)
		weekday = 7;
}

uint8_t daysInMonth()
{
	if (month == 2) {
		if ((year & 0x03) == 0) {
			return 29;
		} else {
			return 28;
		}
	}
	if (month == 4 || month == 6 || month == 9 || month == 11) {
		return 30;
	}
	return 31;
}

void getTime(void)
{
	uint8_t temp;

	I2CRead(0b11010000, 0x02, &temp);
	hour = bd2d(temp);
	I2CRead(0b11010000, 0x01, &temp);
	minute = bd2d(temp);
	I2CRead(0b11010000, 0x00, &temp);
	second = bd2d(temp);

	I2CRead(0b11010000, 0x04, &temp);
	day = bd2d(temp);
	I2CRead(0b11010000, 0x05, &temp);
	month = bd2d(temp);
	I2CRead(0b11010000, 0x06, &temp);
	year = bd2d(temp);

	I2CRead(0b11010000, 0x03, &temp);
	weekday = bd2d(temp);
}

void setTime(void)
{
	if (day > daysInMonth())
		day = daysInMonth();
	I2CWrite(0b11010000, 0x02, d2bd(hour));
	I2CWrite(0b11010000, 0x01, d2bd(minute));
	if (etm == EDIT_SECONDS)
		I2CWrite(0b11010000, 0x00, d2bd(second));

	I2CWrite(0b11010000, 0x04, d2bd(day));
	I2CWrite(0b11010000, 0x05, d2bd(month));
	I2CWrite(0b11010000, 0x06, d2bd(year));

	if (etm >= EDIT_DAY) {
		calcWeekDay();
		I2CWrite(0b11010000, 0x03, d2bd(weekday));
	}
}

void showTime(uint8_t inv)
{
	getTime();
	gdSetXY(4, 0);
	if (etm == EDIT_HOURS)
		gdLoadFont(font_digits_32, 0);
	else
		gdLoadFont(font_digits_32, 1);
	gdWriteString(mkNumString(hour, 2, '0'));
	gdLoadFont(font_digits_32, 1);
	gdWriteString((uint8_t*)"\x7F:\x7F");
	if (etm == EDIT_MINUTES)
		gdLoadFont(font_digits_32, 0);
	else
		gdLoadFont(font_digits_32, 1);
	gdWriteString(mkNumString(minute, 2, '0'));
	gdLoadFont(font_digits_32, 1);
	gdWriteString((uint8_t*)"\x7F:\x7F");
	if (etm == EDIT_SECONDS)
		gdLoadFont(font_digits_32, 0);
	else
		gdLoadFont(font_digits_32, 1);
	gdWriteString(mkNumString(second, 2, '0'));

	gdSetXY(9, 4);
	if (etm == EDIT_DAY)
		gdLoadFont(font_ks0066_ru_24, 0);
	else
		gdLoadFont(font_ks0066_ru_24, 1);
	gdWriteString(mkNumString(day, 2, '0'));
	gdLoadFont(font_ks0066_ru_24, 1);
	gdWriteString((uint8_t*)"\x7F.\x7F");
	if (etm == EDIT_MONTH)
		gdLoadFont(font_ks0066_ru_24, 0);
	else
		gdLoadFont(font_ks0066_ru_24, 1);
	gdWriteString(mkNumString(month, 2, '0'));
	gdLoadFont(font_ks0066_ru_24, 1);
	gdWriteString((uint8_t*)"\x7F.\x7F");
	if (etm == EDIT_YEAR)
		gdLoadFont(font_ks0066_ru_24, 0);
	else
		gdLoadFont(font_ks0066_ru_24, 1);
	gdWriteString(mkNumString(2000 + year, 4, '0'));

	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(19, 7);
	gdWriteStringEeprom(weekdayLabel + 16 * (weekday % 7));

	return;
}

void editTime(void)
{
	if (etm == EDIT_YEAR)
	{
		etm = EDIT_NOEDIT;
		setDisplayTime(3000);
	}
	else
		etm++;
	showTime(0);
}

void incTime(void)
{
	getTime();
	switch (etm) {
	case EDIT_HOURS:
		hour++;
		if (hour > 23)
			hour = 0;
		break;
	case EDIT_MINUTES:
		minute++;
		if (minute > 59)
			minute = 0;
		break;
	case EDIT_SECONDS:
		second++;
		if (second > 59)
			second = 0;
		break;
	case EDIT_DAY:
		day++;
		if (day > daysInMonth())
			day = 1;
		break;
	case EDIT_MONTH:
		month++;
		if (month > 12)
			month = 1;
		break;
	case EDIT_YEAR:
		year++;
		if (year > 99)
			year = 0;
		break;
	default:
		break;
	}
	setTime();
	showTime(0);
}

void decTime(void)
{
	getTime();
	switch (etm) {
	case EDIT_HOURS:
		hour--;
		if (hour < 0)
			hour = 23;
		break;
	case EDIT_MINUTES:
		minute--;
		if (minute < 0)
			minute = 59;
		break;
	case EDIT_SECONDS:
		second--;
		if (second < 0)
			second = 59;
		break;
	case EDIT_DAY:
		day--;
		if (day < 1)
			day = daysInMonth();
		break;
	case EDIT_MONTH:
		month--;
		if (month < 1)
			month = 12;
		break;
	case EDIT_YEAR:
		year--;
		if (year < 0)
			year = 99;
		break;
	default:
		break;
	}
	setTime();
	showTime(0);
}
