#ifndef TUNER_H
#define TUNER_H

#include <inttypes.h>

/* Tuner type selection */
#if !defined(_TEA5767) && !defined(_TUX032) && !defined(_LM7001) && !defined(_RDA5807)
#define _TEA5767
#endif

#if defined(_TEA5767)
#include "tea5767.h"
#elif defined(_TUX032)
#include "tux032.h"
#elif defined(_LM7001)
#include "lm7001.h"
#elif defined(_RDA5807)
#include "rda5807.h"
#endif

#define FM_COUNT		50

#define SEARCH_DOWN		0
#define SEARCH_UP		1

#define FM_MONO			1
#define FM_STEREO		0

#define FM_FREQ_MIN		8750
#define FM_FREQ_MAX		10800

void tunerInit();


void tunerSetFreq(uint16_t freq);
uint16_t tunerGetFreq();






void tunerReadStatus();
void tunerSwitchMono();
uint8_t tunerStereo();
uint8_t tunerLevel();

uint8_t tunerStationNum(uint16_t freq);
void tunerNextStation(uint8_t direction);
void tunerLoadStation(uint8_t num);
void tunerStoreStation(void);





void tunerPowerOn(void);
void tunerPowerOff(void);

#endif /* TUNER_H */
