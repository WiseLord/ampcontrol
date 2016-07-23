#ifndef TUNER_H
#define TUNER_H

#include <inttypes.h>

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

typedef struct {
	tunerIC ic;
	uint8_t ctrl;
	uint8_t step1;
	uint8_t step2;
	uint16_t fMin;
	uint16_t fMax;
	uint16_t freq;
	uint8_t mono;
	uint8_t rds;
	int8_t volume;
	uint8_t mute;
	uint8_t bass;
} Tuner_type;

extern Tuner_type tuner;

#define FM_BAND_DIV_FREQ	7600

#define FM_COUNT			62
#define FM_FAV_COUNT		10

#define SEARCH_UP			1
#define SEARCH_DOWN			-1

void tunerInit(void);

void tunerSetFreq();

void tunerChangeFreq(int8_t mult);

void tunerReadStatus(void);
void tunerSetMono(uint8_t value);
void tunerSetRDS(uint8_t value);
uint8_t tunerStereo(void);
uint8_t tunerLevel(void);

uint8_t tunerStationNum(void);
void tunerNextStation(int8_t direction);
void tunerLoadStation(uint8_t num);
void tunerStoreStation(void);

uint8_t tunerFavStationNum(void);
void tunerLoadFavStation(uint8_t num);
void tunerStoreFavStation(uint8_t num);

void tunerSetVolume(int8_t value);
void tunerSetMute(uint8_t value);
void tunerSetBass(uint8_t value);

void tunerPowerOn(void);
void tunerPowerOff(void);

#endif /* TUNER_H */
