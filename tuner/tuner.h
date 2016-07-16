#ifndef TUNER_H
#define TUNER_H

#include <inttypes.h>

#ifdef _TEA5767
#include "tea5767.h"
#endif
#ifdef _RDA580X
#include "rda580x.h"
#endif
#ifdef _TUX032
#include "tux032.h"
#endif
#ifdef _LM7001
#include "lm7001.h"
#endif

typedef enum {
	TUNER_NO = 0,
	TUNER_TEA5767,
	TUNER_RDA5807,
	TUNER_TUX032,
	TUNER_LM7001,
	TUNER_RDA5802,
	TUNER_RDA5807_DF,

	TUNER_END
} tunerIC;

#define FM_COUNT			50

#define SEARCH_UP			1
#define SEARCH_DOWN			-1

void tunerInit(void);
tunerIC tunerGetType(void);

void tunerSetFreq(uint16_t freq);
uint16_t tunerGetFreq(void);
uint16_t tunerGetFreqMin(void);
uint16_t tunerGetFreqMax(void);
uint8_t tunerGetMono(void);

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
void tunerSetVolume(int8_t value);

void tunerPowerOn(void);
void tunerPowerOff(void);

#endif /* TUNER_H */
