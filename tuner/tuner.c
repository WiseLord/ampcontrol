#include "tuner.h"

#include <avr/eeprom.h>
#include "../eeprom.h"

//uint8_t *bufFM;
#if defined(_RDA580X) || defined(_TEA5767) || defined(_TUX032) || defined(_SI470X)
uint8_t tunerRdbuf[TUNER_RDBUF_SIZE];
#endif

Tuner_type tuner;

void tunerInit()
{
    eeprom_read_block(&tuner, (void *)EEPROM_FM_TUNER, EEPROM_FM_TUNER_SIZE);

    // If defined only tuner, use it despite on eeprom value
#if   !defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001) && !defined(_LC72131) && !defined(_SI470X)
    tuner.ic = TUNER_NO;
#elif  defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001) && !defined(_LC72131) && !defined(_SI470X)
    tuner.ic = TUNER_TEA5767;
#elif !defined(_TEA5767) &&  defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001) && !defined(_LC72131) && !defined(_SI470X)
    if (tuner.ic != TUNER_RDA5802)
        tuner.ic = TUNER_RDA5807;
#elif !defined(_TEA5767) && !defined(_RDA580X) &&  defined(_TUX032) && !defined(_LM7001) && !defined(_LC72131) && !defined(_SI470X)
    tuner.ic = TUNER_TUX032;
#elif !defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) &&  defined(_LM7001) && !defined(_LC72131) && !defined(_SI470X)
    tuner.ic = TUNER_LM7001;
#elif !defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001) &&  defined(_LC72131) && !defined(_SI470X)
    tuner.ic = TUNER_LC72131;
#elif !defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001) && !defined(_LC72131) &&  defined(_SI470X)
    tuner.ic = TUNER_SI470X;
#else
    if (tuner.ic >= TUNER_END)
        tuner.ic = TUNER_NO;
#endif

    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        tea5767Init();
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xInit();
        break;
#endif
#ifdef _TUX032
    case TUNER_TUX032:
        tux032Init();
        break;
#endif
#ifdef _LM7001
    case TUNER_LM7001:
        lm7001Init();
        break;
#endif
#ifdef _LC72131
    case TUNER_LC72131:
        lc72131Init();
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xInit();
        break;
#endif
    default:
        break;
    }

    tunerSetFreq();
}

void tunerSetFreq()
{
    if (tuner.freq < tuner.fMin)
        tuner.freq = tuner.fMin;
    else if (tuner.freq > tuner.fMax)
        tuner.freq = tuner.fMax;

    tuner.rdFreq = tuner.freq;

    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        tea5767SetFreq();
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xSetFreq();
        break;
#endif
#ifdef _TUX032
    case TUNER_TUX032:
        tux032SetFreq();
        break;
#endif
#ifdef _LM7001
    case TUNER_LM7001:
        lm7001SetFreq();
        break;
#endif
#ifdef _LC72131
    case TUNER_LC72131:
        lc72131SetFreq();
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSetFreq();
        break;
#endif
    default:
        break;
    }

#ifdef _RDS
    rdsDisable(); // Clear RDS buffer
#endif
}

void tunerChangeFreq(int8_t mult)
{
    tuner.freq = tuner.rdFreq;
    if ((tuner.freq > FM_BAND_DIV_FREQ) || (mult > 0 && tuner.freq == FM_BAND_DIV_FREQ))
        tuner.freq += tuner.step2 * mult;
    else
        tuner.freq += tuner.step1 * mult;

    tunerSetFreq();
}

void tunerReadStatus()
{
    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        tea5767ReadStatus();
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xReadStatus();
        break;
#endif
#ifdef _TUX032
    case TUNER_TUX032:
        tux032ReadStatus();
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xReadStatus();
        break;
#endif
    default:
        break;
    }
}

void tunerSetMono(uint8_t value)
{
    tuner.mono = value;

    switch (tuner.ic) {
    case TUNER_TEA5767:
        tunerSetFreq();
        break;
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xSetMono(value);
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSetMono(value);
        break;
#endif
    default:
        tuner.mono = 0;
        break;
    }
}

#ifdef _RDS
void tunerSetRDS(uint8_t value)
{
    tuner.rds = value;

    switch (tuner.ic) {
#ifdef _RDA580X
    case TUNER_RDA5807:
        rda580xSetRds(value);
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSetRds(value);
        break;
#endif
    default:
        tuner.rds = 0;
        break;
    }
}
#endif

uint8_t tunerStereo()
{
    uint8_t ret = !tuner.mono;

    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        ret = TEA5767_BUF_STEREO(tunerRdbuf);
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        ret = RDA5807_BUF_STEREO(tunerRdbuf);
        break;
#endif
#ifdef _TUX032
    case TUNER_TUX032:
        ret = !TUX032_BUF_STEREO(tunerRdbuf);
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        ret = SI470X_BUF_STEREO(tunerRdbuf);
        break;
#endif
    default:
        break;
    }

    return ret;
}

uint8_t tunerLevel()
{
    uint8_t ret = 0;

    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        ret = (tunerRdbuf[3] & TEA5767_LEV_MASK) >> 4;
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        ret = (tunerRdbuf[2] & RDA580X_RSSI) >> 1;
        if (ret < 30)
            ret = 0;
        else
            ret = (ret - 30) >> 1;
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        ret = (tunerRdbuf[1] / 2);
        break;
#endif
    default:
        if (tunerStereo())
            ret = 13;
        else
            ret = 3;
        break;
    }

    return ret;
}

// Find station number (1..62) in EEPROM
uint8_t tunerStationNum()
{
    uint8_t i;

    for (i = 0; i < FM_COUNT; i++)
        if (eeprom_read_word((uint16_t *)EEPROM_STATIONS + i) == tuner.rdFreq)
            return i + 1;

    return 0;
}

// Find favourite station number (1..10) in EEPROM
uint8_t tunerFavStationNum()
{
    uint8_t i;

    for (i = 0; i < FM_FAV_COUNT; i++)
        if (eeprom_read_word((uint16_t *)EEPROM_FAV_STATIONS + i) == tuner.rdFreq)
            return i + 1;

    return 0;
}

// Find nearest next/prev stored station
void tunerNextStation(int8_t direction)
{
    uint8_t i;
    uint16_t freqCell;
    uint16_t freq;

    tuner.freq = tuner.rdFreq;
    freq = tuner.freq;

    for (i = 0; i < FM_COUNT; i++) {
        freqCell = eeprom_read_word((uint16_t *)EEPROM_STATIONS + i);
        if (freqCell != 0xFFFF) {
            if (direction == SEARCH_UP) {
                if (freqCell > tuner.freq) {
                    freq = freqCell;
                    break;
                }
            } else {
                if (freqCell < tuner.freq) {
                    freq = freqCell;
                } else {
                    break;
                }
            }
        }
    }

    tuner.freq = freq;

    tunerSetFreq();
}

// Load station by number
void tunerLoadStation(uint8_t num)
{
    uint16_t freq = eeprom_read_word((uint16_t *)EEPROM_STATIONS + num);

    if (freq >= tuner.fMin && freq <= tuner.fMax) {
        tuner.freq = freq;
        tunerSetFreq();
    }
}

// Load favourite station by number
void tunerLoadFavStation(uint8_t num)
{
    uint16_t freq = eeprom_read_word((uint16_t *)EEPROM_FAV_STATIONS + num);

    if (freq >= tuner.fMin && freq <= tuner.fMax) {
        tuner.freq = freq;
        tunerSetFreq();
    }
}

// Load favourite station by number
void tunerStoreFavStation(uint8_t num)
{
    uint16_t freq = eeprom_read_word((uint16_t *)EEPROM_FAV_STATIONS + num);

    if (freq == tuner.freq)
        eeprom_update_word((uint16_t *)EEPROM_FAV_STATIONS + num, 0);
    else
        eeprom_update_word((uint16_t *)EEPROM_FAV_STATIONS + num, tuner.freq);
}

// Save/delete station from eeprom
void tunerStoreStation()
{
    uint8_t i, j;
    uint16_t freqCell;
    uint16_t freq;

    freq = tuner.freq;

    for (i = 0; i < FM_COUNT; i++) {
        freqCell = eeprom_read_word((uint16_t *)EEPROM_STATIONS + i);
        if (freqCell < freq)
            continue;
        if (freqCell == freq) {
            for (j = i; j < FM_COUNT; j++) {
                if (j == FM_COUNT - 1)
                    freqCell = 0xFFFF;
                else
                    freqCell = eeprom_read_word((uint16_t *)EEPROM_STATIONS + j + 1);
                eeprom_update_word((uint16_t *)EEPROM_STATIONS + j, freqCell);
            }
            break;
        } else {
            for (j = i; j < FM_COUNT; j++) {
                freqCell = eeprom_read_word((uint16_t *)EEPROM_STATIONS + j);
                eeprom_update_word((uint16_t *)EEPROM_STATIONS + j, freq);
                freq = freqCell;
            }
            break;
        }
    }
}

void tunerSetVolume(int8_t value)
{
    tuner.volume = value;

    switch (tuner.ic) {
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xSetVolume(value);
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSetVolume(value);
        break;
#endif
    default:
        break;
    }
}

void tunerSetMute(uint8_t value)
{
    tuner.mute = value;

    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        tea5767SetMute();
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xSetMute(value);
        break;
#endif
#ifdef _TUX032
    case TUNER_TUX032:
        tux032SetMute();
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSetMute(value);
        break;
#endif
    default:
        break;
    }
}

void tunerSetBass(uint8_t value)
{
    switch (tuner.ic) {
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        tuner.bass = value;
        rda580xSetBass(value);
        break;
#endif
    default:
        tuner.bass = 0;
        break;
    }
}

void tunerPowerOn()
{
    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        tea5767PowerOn();
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xSetPower(1);
        break;
#endif
#ifdef _TUX032
    case TUNER_TUX032:
        tux032PowerOn();
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSetPower(1);
        break;
#endif
    default:
        break;
    }

    tunerSetFreq();
}

void tunerPowerOff()
{
    tuner.freq = tuner.rdFreq;
    eeprom_update_block(&tuner, (void *)EEPROM_FM_TUNER, EEPROM_FM_TUNER_SIZE);

    switch (tuner.ic) {
#ifdef _TEA5767
    case TUNER_TEA5767:
        tea5767PowerOff();
        break;
#endif
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xSetPower(0);
        break;
#endif
#ifdef _TUX032
    case TUNER_TUX032:
        tux032PowerOff();
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSetPower(0);
        break;
#endif
    default:
        break;
    }
}

void tunerSeek(int8_t direction)
{
    switch (tuner.ic) {
#ifdef _RDA580X
    case TUNER_RDA5807:
    case TUNER_RDA5802:
        rda580xSeek(direction);
        break;
#endif
#ifdef _SI470X
    case TUNER_SI470X:
        si470xSeek(direction);
        break;
#endif
    default:
        tunerChangeFreq(direction * 10);
        break;
    }
}
