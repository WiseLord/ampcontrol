#ifndef ALARM_H
#define ALARM_H

#include <inttypes.h>

#define DS1307_A0_HOUR		0x08
#define DS1307_A0_MIN		0x09
#define DS1307_A0_INPUT		0x0A
#define DS1307_A0_WDAY		0x0B

int8_t getAlarm(uint8_t am);
uint8_t getEam(void);
int8_t *readAlarm(void);
void stopEditAlarm(void);
uint8_t isEAM(void);
void editAlarm(void);
void changeAlarm(int diff);

#endif // ALARM_H
