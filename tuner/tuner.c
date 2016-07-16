#include "tuner.h"

#include <avr/eeprom.h>
#include "../eeprom.h"

uint8_t *bufFM;
static tunerIC _tuner;

static uint16_t _freq, _fMin, _fMax;
static uint8_t _mono;
static uint8_t _step1, _step2;

void tunerInit(void)
{
	_tuner = eeprom_read_byte((uint8_t*)EEPROM_FM_TUNER);
	_freq = eeprom_read_word((uint16_t*)EEPROM_FM_FREQ);
	_fMin = eeprom_read_word((uint16_t*)EEPROM_FM_FREQ_MIN);
	_fMax = eeprom_read_word((uint16_t*)EEPROM_FM_FREQ_MAX);
	_mono = eeprom_read_byte((uint8_t*)EEPROM_FM_MONO);
	_step1 = eeprom_read_byte((uint8_t*)EEPROM_FM_STEP1);
	_step2 = eeprom_read_byte((uint8_t*)EEPROM_FM_STEP2);

	if (_tuner >= TUNER_END)
		_tuner = TUNER_NO;

	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		tea5767Init(eeprom_read_byte((uint8_t*)EEPROM_FM_CTRL));
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
		rda580xInit(RDA580X_RDA5807);
		break;
	case TUNER_RDA5802:
		rda580xInit(RDA580X_RDA5802);
		break;
	case TUNER_RDA5807_DF:
		rda580xInit(RDA580X_RDA5807_DF);
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
	default:
		break;
	}

	tunerSetFreq(_freq);

	return;
}

tunerIC tunerGetType(void)
{
	return _tuner;
}

void tunerSetFreq(uint16_t freq)
{
	if (freq > _fMax)
		freq = _fMax;
	if (freq < _fMin)
		freq = _fMin;

	_freq = freq;

	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		tea5767SetFreq(_freq, _mono);
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		rda580xSetFreq(_freq, _mono);
		break;
#endif
#ifdef _TUX032
	case TUNER_TUX032:
		tux032SetFreq(_freq);
		break;
#endif
#ifdef _LM7001
	case TUNER_LM7001:
		lm7001SetFreq(_freq);
		break;
#endif
	default:
		break;
	}

	return;
}

uint16_t tunerGetFreq(void)
{
	return _freq;
}

uint16_t tunerGetFreqMin(void)
{
	return _fMin;
}

uint16_t tunerGetFreqMax(void)
{
	return _fMax;
}

uint8_t tunerGetMono(void)
{
	return _mono;
}

void tunerChangeFreq(int8_t mult)
{
	uint16_t freq;

	if (mult > 0) {
		if (_freq >= 7600)
			freq = _freq + _step2 * mult;
		else
			freq = _freq + _step1 * mult;
	} else {
		if (_freq <= 7600)
			freq = _freq + _step1 * mult;
		else
			freq = _freq + _step2 * mult;
	}

	tunerSetFreq(freq);

	return;
}

void tunerReadStatus(void)
{
	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		bufFM = tea5767ReadStatus();
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		bufFM = rda580xReadStatus();
		break;
#endif
#ifdef _TUX032
	case TUNER_TUX032:
		bufFM = tux032ReadStatus();
		break;
#endif
	default:
		break;
	}

	return;
}

void tunerSwitchMono(void)
{
	_mono = !_mono;

	switch (_tuner) {
	case TUNER_TEA5767:
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		tunerSetFreq(tunerGetFreq());
		break;
	default:
		break;
	}

	return;
}

uint8_t tunerStereo(void)
{
	uint8_t ret = 1;

	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		ret = TEA5767_BUF_STEREO(bufFM) && !_mono;
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		ret = RDA5807_BUF_STEREO(bufFM) && !_mono;
		break;
#endif
#ifdef _TUX032
	case TUNER_TUX032:
		ret = !TUX032_BUF_STEREO(bufFM);
		break;
#endif
	default:
		break;
	}

	return ret;
}

uint8_t tunerLevel(void)
{
	uint8_t ret = 0;

	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		ret = (bufFM[3] & TEA5767_LEV_MASK) >> 4;
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		ret = (bufFM[2] & RDA5807_RSSI) >> 1;
		if (ret < 24)
			ret = 0;
		else
			ret = (ret - 24) >> 1;
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

/* Find station number (1..50) in EEPROM */
uint8_t tunerStationNum(void)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word((uint16_t*)EEPROM_STATIONS + i) == _freq)
			return i + 1;

	return 0;
}

/* Find favourite station number (1..10) in EEPROM */
uint8_t tunerFavStationNum(void)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word((uint16_t*)EEPROM_FAV_STATIONS + i) == _freq)
			return i + 1;

	return 0;
}

/* Find nearest next/prev stored station */
void tunerNextStation(int8_t direction)
{
	uint8_t i;
	uint16_t freqCell;
	uint16_t freqFound = _freq;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + i);
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

/* Load station by number */
void tunerLoadStation(uint8_t num)
{
	uint16_t freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + num);

	if (freqCell != 0xFFFF)
		tunerSetFreq(freqCell);

	return;
}

/* Load favourite station by number */
void tunerLoadFavStation(uint8_t num)
{
	if (eeprom_read_word((uint16_t*)EEPROM_FAV_STATIONS + num) != 0)
		tunerSetFreq(eeprom_read_word((uint16_t*)EEPROM_FAV_STATIONS + num));

	return;
}

/* Load favourite station by number */
void tunerStoreFavStation(uint8_t num)
{
	if (eeprom_read_word((uint16_t*)EEPROM_FAV_STATIONS + num) == _freq)
		eeprom_update_word((uint16_t*)EEPROM_FAV_STATIONS + num, 0);
	else
		eeprom_update_word((uint16_t*)EEPROM_FAV_STATIONS + num, _freq);

	return;
}

/* Save/delete station from eeprom */
void tunerStoreStation(void)
{
	uint8_t i, j;
	uint16_t freqCell;
	uint16_t freq;

	freq = _freq;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + i);
		if (freqCell < freq)
			continue;
		if (freqCell == freq) {
			for (j = i; j < FM_COUNT; j++) {
				if (j == FM_COUNT - 1)
					freqCell = 0xFFFF;
				else
					freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + j + 1);
				eeprom_update_word((uint16_t*)EEPROM_STATIONS + j, freqCell);
			}
			break;
		} else {
			for (j = i; j < FM_COUNT; j++) {
				freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + j);
				eeprom_update_word((uint16_t*)EEPROM_STATIONS + j, freq);
				freq = freqCell;
			}
			break;
		}
	}

	return;
}

void tunerSetMute(uint8_t mute)
{
	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		tea5767SetMute(mute);
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		rda580xSetMute(mute);
		break;
#endif
#ifdef _TUX032
	case TUNER_TUX032:
		tux032SetMute(mute);
		break;
#endif
	default:
		break;
	}

	return;
}

void tunerSetVolume(int8_t value)
{
	switch (_tuner) {
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		rda580xSetVolume(value);
		break;
#endif
	default:
		break;
	}

	return;
}

void tunerPowerOn(void)
{
	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		tea5767PowerOn();
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		rda580xPowerOn();
		break;
#endif
#ifdef _TUX032
	case TUNER_TUX032:
		tux032PowerOn();
		break;
#endif
	default:
		break;
	}

	tunerSetFreq(_freq);

	return;
}

void tunerPowerOff(void)
{
	eeprom_update_word((uint16_t*)EEPROM_FM_FREQ, _freq);
	eeprom_update_byte((uint8_t*)EEPROM_FM_MONO, _mono);
	eeprom_update_byte((uint8_t*)EEPROM_FM_TUNER, _tuner);

	switch (_tuner) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		tea5767PowerOff();
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		rda580xPowerOff();
		break;
#endif
#ifdef _TUX032
	case TUNER_TUX032:
		tux032PowerOff();
		break;
#endif
	default:
		break;
	}

	return;
}
