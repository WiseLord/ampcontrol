#ifndef ALARM_H
#define ALARM_H

#include <inttypes.h>

enum {
	ALARM_HOUR = 0,
	ALARM_MIN,
	ALARM_INPUT,
	ALARM_WDAY,
	ALARM_EAM,
};

typedef struct {
	int8_t hour;
	int8_t min;
	int8_t input;
	int8_t wday;
	uint8_t eam;
} ALARM_type;

extern ALARM_type alarm0;

#define ALARM_NOEDIT		0xFF

void alarmInit(void);
void alarmSave(void);

void alarmNextEditParam(void);
void alarmChangeTime(int diff);

#endif // ALARM_H
