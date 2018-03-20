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
#ifdef _LC72131
#include "lc72131.h"
#endif
#ifdef _SI470X
#include "si470x.h"
#endif
#ifdef _RDS
#include "rds.h"
#endif

typedef enum {
    TUNER_NO = 0,
    TUNER_TEA5767,
    TUNER_RDA5807,
    TUNER_TUX032,
    TUNER_LM7001,
    TUNER_RDA5802,
    TUNER_RDA5807_DF,
    TUNER_LC72131,
    TUNER_SI470X,

    TUNER_END
} tunerIC;

typedef struct {
    // EEPROM part
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
    uint8_t bass;
    // LIVE part
    uint16_t rdFreq;
    uint8_t mute;
} Tuner_type;

extern Tuner_type tuner;

#if defined(_RDA580X)
#define TUNER_RDBUF_SIZE    RDA5807_RDBUF_SIZE
#elif defined(_SI470X)
#define TUNER_RDBUF_SIZE    SI470X_RDBUF_SIZE
#elif defined(_TEA5767)
#define TUNER_RDBUF_SIZE    TEA5767_RDBUF_SIZE
#elif defined(_TUX032)
#define TUNER_RDBUF_SIZE    TUX032_RDBUF_SIZE
#endif

#if defined(_RDA580X) || defined(_TEA5767) || defined(_TUX032) || defined(_SI470X)
extern uint8_t tunerRdbuf[];
#endif

#define FM_BAND_DIV_FREQ    7600

#define FM_COUNT            62
#define FM_FAV_COUNT        10

#define SEARCH_UP           1
#define SEARCH_DOWN         -1

void tunerInit();

void tunerSetFreq();

void tunerChangeFreq(int8_t mult);

void tunerReadStatus();
void tunerSetMono(uint8_t value);
#ifdef _RDS
void tunerSetRDS(uint8_t value);
#endif
uint8_t tunerStereo();
uint8_t tunerLevel();

uint8_t tunerStationNum();
void tunerNextStation(int8_t direction);
void tunerLoadStation(uint8_t num);
void tunerStoreStation();

uint8_t tunerFavStationNum();
void tunerLoadFavStation(uint8_t num);
void tunerStoreFavStation(uint8_t num);

void tunerSetVolume(int8_t value);
void tunerSetMute(uint8_t value);
void tunerSetBass(uint8_t value);

void tunerPowerOn();
void tunerPowerOff();

void tunerSeek(int8_t direction);

#endif // TUNER_H
