#ifndef DS1307_H
#define DS1307_H

#include <inttypes.h>

#define DS1307_ADDR		0b11010000

/* DS1307 memory */
#define DS1307_SECONDS	0
#define DS1307_MINUTES	1
#define DS1307_HOURS	2
#define DS1307_WEEKDAY	3
#define DS1307_DAY		4
#define DS1307_MONTH	5
#define DS1307_YEAR		6

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
