#ifndef DS1307_H
#define DS1307_H

#include <inttypes.h>

#define DS1307_ADDR			0b11010000

#define BD2D(x)				((x >> 4) * 10 + (x & 0x0F))
#define D2BD(x)				(((x / 10) << 4) + (x % 10))

#define DS1307_SEC			0x00
#define DS1307_MIN			0x01
#define DS1307_HOUR			0x02
#define DS1307_WDAY			0x03
#define DS1307_DATE			0x04
#define DS1307_MONTH		0x05
#define DS1307_YEAR			0x06
#define DS1307_CTRL			0x07

#define DS1307_A0_SEC		0x08
#define DS1307_A0_MIN		0x09
#define DS1307_A0_HOUR		0x0A
#define DS1307_A0_WDAY		0x0B
#define DS1307_A0_INPUT		0x0C

#define NOEDIT				0xFF

int8_t getTime(uint8_t tm);
int8_t getAlarm(uint8_t tm);

uint8_t getEtm(void);
uint8_t getEam(void);

int8_t *readTime(void);
int8_t *readAlarm(void);

void stopEditTime(void);
void stopEditAlarm(void);

uint8_t isETM(void);
uint8_t isEAM(void);

void editTime(void);
void editAlarm(void);

void changeTime(int diff);
void changeAlarm(int diff);

#endif /* DS1307_H */
