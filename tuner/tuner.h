#ifndef TUNER_H
#define TUNER_H

#include <inttypes.h>
#include "tea5767.h"
#include "rda5807.h"
#include "tux032.h"

typedef enum {
	TUNER_NO = 0,
	TUNER_TEA5767,
	TUNER_RDA5807,
	TUNER_TUX032,
	TUNER_END
} tunerIC;

#define FM_COUNT			64

#define SEARCH_UP			1
#define SEARCH_DOWN			-1

#define FM_MONO				1
#define FM_STEREO			0

#define FM_FREQ_MIN			7600
#define FM_FREQ_MAX			10800

void tunerInit(void);
tunerIC tunerGetType(void);

void tunerSetFreq(uint16_t freq);
uint16_t tunerGetFreq(void);

void tunerChangeFreq(int8_t mult);

void tunerReadStatus(void);
void tunerSwitchMono(void);
uint8_t tunerStereo(void);
uint8_t tunerLevel(void);

uint8_t tunerStationNum(void);
void tunerNextStation(int8_t direction);
void tunerLoadStation(uint8_t num);
void tunerStoreStation(void);

void tunerPowerOn(void);
void tunerPowerOff(void);

#endif /* TUNER_H */
