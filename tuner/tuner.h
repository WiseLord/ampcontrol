#ifndef TUNER_H
#define TUNER_H

#include <inttypes.h>

#if !defined(TEA5767) && !defined(TUX032) && !defined(LM7001) && !defined(RDA5807)
#define RDA5807
#endif

#if defined(TEA5767)
#include "tea5767.h"
#elif defined(RDA5807)
#include "rda5807.h"
#elif defined(TUX032)
#include "tux032.h"
#elif defined(LM7001)
#include "lm7001.h"
#endif

#define FM_COUNT            50

#define SEARCH_UP           1
#define SEARCH_DOWN         -1

#define FM_MONO             1
#define FM_STEREO           0

#define FM_FREQ_MIN         8700
#define FM_FREQ_MAX         10800

void tunerInit();

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

uint8_t tunerFavStationNum(void);
void tunerLoadFavStation(uint8_t num);
void tunerStoreFavStation(uint8_t num);

void tunerSetMute(uint8_t mute);

void tunerPowerOn(void);
void tunerPowerOff(void);

#endif // TUNER_H
