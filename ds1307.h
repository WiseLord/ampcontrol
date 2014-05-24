#ifndef DS1307_H
#define DS1307_H

#include <inttypes.h>

#define DS1307_ADDR		0b11010000

#define BD2D(x)			((x >> 4) * 10 + (x & 0x0F))
#define D2BD(x)			(((x / 10) << 4) + (x % 10))

typedef enum {
	SEC   = 0,
	MIN   = 1,
	HOUR  = 2,
	WEEK  = 3,
	DAY   = 4,
	MONTH = 5,
	YEAR  = 6,
	NOEDIT = 7
} timeMode;

int8_t getTime(timeMode tm);
timeMode getEtm();

int8_t *readTime(void);
void stopEditTime(void);
uint8_t isETM(void);
void editTime(void);
void changeTime(int diff);

#endif /* DS1307_H */
