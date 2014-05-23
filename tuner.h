#ifndef TUNER_H
#define TUNER_H

#include <inttypes.h>

/* Tuner type selection */
#if !defined(TEA5767) && !defined(TUX032)
#define TEA5767
#endif

#if defined(TEA5767)
#include "tuner/tea5767.h"
#elif defined(TUX032)
#include "tuner/tux032.h"
#endif

#define FM_COUNT		64

#define SEARCH_DOWN		0
#define SEARCH_UP		1

#define FM_FREQ_MIN		8750
#define FM_FREQ_MAX		10800

#if defined(TEA5767)

#define tunerInit() tea5767Init()
#define tunerSetFreq(freq) tea5767SetFreq(freq)

#elif defined(TUX032)

#define tunerInit() tux032Init()
#define tunerSetFreq(freq)	tux032SetFreq(freq); freqFM = (freq);

#endif

void tunerSearch(uint16_t freq, uint8_t direction);
void tunerReadStatus();
uint16_t tunerFreqAvail();
uint8_t tunerReady();
uint8_t tunerStereo();
uint8_t tunerLevel();
void fineTune(uint16_t *freq);

uint8_t stationNum(uint16_t freq);
void scanStoredFreq(uint16_t freq, uint8_t direction);
void loadStation(uint8_t num);
void storeStation(uint16_t freq);

void loadTunerParams(uint16_t *freq);
void saveTunerParams(uint16_t freq);

#endif /* TUNER_H */
