#ifndef TUNER_H
#define TUNER_H

#include <inttypes.h>

/* Tuner type selection */
#if !defined(TEA5767) && !defined(TUX032) && !defined(LM7001) && !defined(RDA5807) && !defined(NOTUNER)
#define RDA5807
#endif

#if defined(TEA5767)
#include "tuner/tea5767.h"
#elif defined(TUX032)
#include "tuner/tux032.h"
#elif defined(LM7001)
#include "tuner/lm7001.h"
#elif defined(RDA5807)
#include "tuner/rda5807.h"
#endif

#define FM_COUNT		64

#define SEARCH_DOWN		0
#define SEARCH_UP		1

#define FM_MONO			1
#define FM_STEREO		0

#define FM_FREQ_MIN		8750
#define FM_FREQ_MAX		10800

#if defined(TEA5767)
#define tunerInit(); tea5767Init();
#elif defined(TUX032)
#define tunerInit(); tux032Init();
#elif defined(LM7001)
#define tunerInit(); lm7001Init();
#elif defined(RDA5807)
#define tunerInit(); rda5807Init();
#endif

void tunerSetFreq(uint16_t freq);

void tunerReadStatus();
uint16_t tunerGetFreq();
void tunerSwitchMono();
uint8_t tunerStereo();
uint8_t tunerLevel();

uint8_t stationNum(uint16_t freq);
void scanStoredFreq(uint16_t freq, uint8_t direction);
void loadStation(uint8_t num);
void storeStation(uint16_t freq);

void loadTunerParams(uint16_t *freq);
void saveTunerParams(uint16_t freq);

#endif /* TUNER_H */
