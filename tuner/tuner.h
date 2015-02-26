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

#define SEARCH_DOWN			0
#define SEARCH_UP			1

#define FM_MONO				1
#define FM_STEREO			0

#define FM_FREQ_MIN			RDA5807_FREQ_MIN
#define FM_FREQ_MAX			RDA5807_FREQ_MAX

#define FM_STEP				10

void tunerInit(void);

void tunerSetFreq(uint16_t freq);
uint16_t tunerGetFreq();

void tunerIncFreq(uint8_t mult);
void tunerDecFreq(uint8_t mult);

void tunerReadStatus();
void tunerSwitchMono();
uint8_t tunerStereo();
uint8_t tunerLevel();

void tunerChangeFreq(int8_t mult);

uint8_t stationNum(void);
void scanStoredFreq(uint8_t direction);
void loadStation(uint8_t num);
void storeStation(void);

void loadTunerParams(void);
void saveTunerParams(void);

void setTunerParams(void);

#endif /* TUNER_H */
