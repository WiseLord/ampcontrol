#include "tuner.h"

#include <avr/eeprom.h>
#include "../eeprom.h"
#include "../pins.h"

uint8_t *bufFM;

static uint16_t _freq;
static uint8_t _mono;
static uint8_t _step;
static uint8_t _ctrl;

void tunerInit()
{
    _ctrl = eeprom_read_byte((uint8_t *)EEPROM_FM_CTRL);
    _freq = eeprom_read_word((uint16_t *)EEPROM_FM_FREQ);
    _mono = eeprom_read_byte((uint8_t *)EEPROM_FM_MONO);
    _step = eeprom_read_byte((uint8_t *)EEPROM_FM_STEP2);

#if defined(TEA5767)
    tea5767Init(_ctrl);
#elif defined(RDA5807)
    rda5807Init();
#elif defined(TUX032)
    tux032Init();
#elif defined(LM7001)
    lm7001Init();
#endif

    return;
}

void tunerSetFreq(uint16_t freq)
{
    if (freq > FM_FREQ_MAX)
        freq = FM_FREQ_MAX;
    if (freq < FM_FREQ_MIN)
        freq = FM_FREQ_MIN;

    _freq = freq;

#if defined(TEA5767)
    tea5767SetFreq(_freq, _mono);
#elif defined(RDA5807)
    rda5807SetFreq(_freq, _mono);
#elif defined(TUX032)
    tux032SetFreq(_freq);
#elif defined(LM7001)
    lm7001SetFreq(_freq);
#endif

    return;
}

uint16_t tunerGetFreq(void)
{
    return _freq;
}

void tunerChangeFreq(int8_t mult)
{
    tunerSetFreq(_freq + _step * mult);

    return;
}

void tunerReadStatus(void)
{
#if defined(TEA5767)
    bufFM = tea5767ReadStatus();
#elif defined(RDA5807)
    bufFM = rda5807ReadStatus();
#elif defined(TUX032)
    bufFM = tux032ReadStatus();
#endif

    return;
}

void tunerSwitchMono(void)
{
    _mono = !_mono;

#if defined(TEA5767) || defined(RDA5807)
    tunerSetFreq(tunerGetFreq());
#endif

    return;
}

uint8_t tunerStereo(void)
{
    uint8_t ret = 1;

#if defined(TEA5767)
    ret = TEA5767_BUF_STEREO(bufFM) && !_mono;
#elif defined(RDA5807)
    ret = RDA5807_BUF_STEREO(bufFM) && !_mono;
#elif defined(TUX032)
    ret = !TUX032_BUF_STEREO(bufFM);
#endif

    return ret;
}

uint8_t tunerLevel(void)
{
    uint8_t ret = 0;

#if defined(TEA5767)
    ret = (bufFM[3] & TEA5767_LEV_MASK) >> 4;
#elif defined(RDA5807)
    uint8_t rawLevel = (bufFM[2] & RDA5807_RSSI) >> 1;
    if (rawLevel < 24)
        ret = 0;
    else
        ret = (rawLevel - 24) >> 1;
#else
    if (tunerStereo())
        ret = 13;
    else
        ret = 3;
#endif

    return ret;
}

// Find station number (1..50) in EEPROM
uint8_t tunerStationNum(void)
{
    uint8_t i;

    for (i = 0; i < FM_COUNT; i++)
        if (eeprom_read_word((uint16_t *)EEPROM_STATIONS + i) == _freq)
            return i + 1;

    return 0;
}

// Find favourite station number (1..10) in EEPROM
uint8_t tunerFavStationNum(void)
{
    uint8_t i;

    for (i = 0; i < FM_COUNT; i++)
        if (eeprom_read_word((uint16_t *)EEPROM_FAV_STATIONS + i) == _freq)
            return i + 1;

    return 0;
}

// Find nearest next/prev stored station
void tunerNextStation(int8_t direction)
{
    uint8_t i;
    uint16_t freqCell;
    uint16_t freqFound = _freq;

    for (i = 0; i < FM_COUNT; i++) {
        freqCell = eeprom_read_word((uint16_t *)EEPROM_STATIONS + i);
        if (freqCell != 0xFFFF) {
            if (direction == SEARCH_UP) {
                if (freqCell > _freq) {
                    freqFound = freqCell;
                    break;
                }
            } else {
                if (freqCell < _freq) {
                    freqFound = freqCell;
                } else {
                    break;
                }
            }
        }
    }

    tunerSetFreq(freqFound);

    return;
}

// Load station by number
void tunerLoadStation(uint8_t num)
{
    uint16_t freqCell = eeprom_read_word((uint16_t *)EEPROM_STATIONS + num);

    if (freqCell != 0xFFFF)
        tunerSetFreq(freqCell);

    return;
}

// Load favourite station by number
void tunerLoadFavStation(uint8_t num)
{
    if (eeprom_read_word((uint16_t *)EEPROM_FAV_STATIONS + num) != 0)
        tunerSetFreq(eeprom_read_word((uint16_t *)EEPROM_FAV_STATIONS + num));

    return;
}

// Load favourite station by number
void tunerStoreFavStation(uint8_t num)
{
    if (eeprom_read_word((uint16_t *)EEPROM_FAV_STATIONS + num) == _freq)
        eeprom_update_word((uint16_t *)EEPROM_FAV_STATIONS + num, 0);
    else
        eeprom_update_word((uint16_t *)EEPROM_FAV_STATIONS + num, _freq);

    return;
}

// Save/delete station from eeprom
void tunerStoreStation(void)
{
    uint8_t i, j;
    uint16_t freqCell;
    uint16_t freq;

    freq = _freq;

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

    return;
}

void tunerSetMute(uint8_t mute)
{
#if defined(TEA5767)
    tea5767SetMute(mute);
#elif defined(RDA5807)
    rda5807SetMute(mute);
#elif defined(TUX032)
    tux032SetMute(mute);
#endif

    return;
}

void tunerPowerOn(void)
{
#if defined(TEA5767)
    tea5767PowerOn();
#elif defined(RDA5807)
    rda5807PowerOn();
#elif defined(TUX032)
    tux032PowerOn();
#endif

    tunerSetFreq(_freq);

    return;
}

void tunerPowerOff(void)
{
    eeprom_update_word((uint16_t *)EEPROM_FM_FREQ, _freq);
    eeprom_update_byte((uint8_t *)EEPROM_FM_MONO, _mono);

#if defined(TEA5767)
    tea5767PowerOff();
#elif defined(RDA5807)
    rda5807PowerOff();
#elif defined(TUX032)
    tux032PowerOff();
#endif

    return;
}
