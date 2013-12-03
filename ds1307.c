#include "ds1307.h"

#include <avr/pgmspace.h>

#include "i2c.h"
#include "ks0108.h"

#define DOW_LENGTH	12
const uint8_t dayOfWeek[][DOW_LENGTH] PROGMEM = {
	"ВОСКРЕСЕНЬЕ",
	"ПОНЕДЕЛЬНИК",
	"ВТОРНИК",
	"СРЕДА",
	"ЧЕТВЕРГ",
	"ПЯТНИЦА",
	"СУББОТА",
};

uint8_t bd2d(uint8_t *temp)
{
	return ((*temp & 0xF0) >> 4) * 10 + (*temp & 0x0F);
}

void showTime()
{
	uint8_t hour, minute, second, day, month, year, weekday;
	uint8_t temp;

	I2CRead(0b11010000, 0x02, &temp);
	hour = bd2d(&temp);
	I2CRead(0b11010000, 0x01, &temp);
	minute = bd2d(&temp);
	I2CRead(0b11010000, 0x00, &temp);
	second = bd2d(&temp);

	I2CRead(0b11010000, 0x04, &temp);
	day = bd2d(&temp);
	I2CRead(0b11010000, 0x05, &temp);
	month = bd2d(&temp);
	I2CRead(0b11010000, 0x06, &temp);
	year = bd2d(&temp);

	I2CRead(0b11010000, 0x04, &temp);
	weekday = bd2d(&temp);

	gdSetXY(0, 0);
	gdWriteStringScaled(mkNumString(hour, 2, '0'), 3, 3);
	gdWriteCharScaled(':', 3, 3);
	gdWriteStringScaled(mkNumString(minute, 2, '0'), 3, 3);
	gdWriteCharScaled(':', 3, 3);
	gdWriteStringScaled(mkNumString(second, 2, '0'), 3, 3);

	gdSetXY(0, 4);
	gdWriteStringScaled(mkNumString(day, 2, '0'), 2, 2);
	gdWriteCharScaled('.', 2, 2);
	gdWriteStringScaled(mkNumString(month, 2, '0'), 2, 2);
	gdWriteCharScaled('.', 2, 2);
	gdWriteStringScaled(mkNumString(year, 2, '0'), 2, 2);

	gdSetXY(0, 7);
	gdWriteStringProgmem(dayOfWeek[weekday % 7]);

	return;
}

