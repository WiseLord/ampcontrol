#ifndef DS1307_H
#define DS1307_H

#include <inttypes.h>

typedef enum {
	EDIT_NOEDIT,
	EDIT_HOURS,
	EDIT_MINUTES,
	EDIT_SECONDS,
	EDIT_DAY,
	EDIT_MONTH,
	EDIT_YEAR,
	EDIT_WEEKDAY,
} editTimeMode;

editTimeMode etm;

void showTime(uint8_t inv);
void editTime(void);
void incTime(void);
void decTime(void);

#endif // DS1307_H
