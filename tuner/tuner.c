#include "tuner.h"

#include <avr/eeprom.h>
#include "../eeprom.h"

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
#ifdef _RDS
#include "rds.h"
#endif

uint8_t *bufFM;

Tuner_type tuner;

void tunerInit(void)
{
	eeprom_read_block(&tuner, (void*)EEPROM_FM_TUNER, sizeof(Tuner_type));

	// If defined only tuner, use it despite on eeprom value
#if   !defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001)
	tuner.ic = TUNER_NO;
#elif  defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001)
	tuner.ic = TUNER_TEA5767;
#elif !defined(_TEA5767) &&  defined(_RDA580X) && !defined(_TUX032) && !defined(_LM7001)
	if (tuner.ic != TUNER_RDA5802 && tuner.ic != TUNER_RDA5807_DF)
		tuner.ic = TUNER_RDA5807;
#elif !defined(_TEA5767) && !defined(_RDA580X) &&  defined(_TUX032) && !defined(_LM7001)
	tuner.ic = TUNER_TUX032;
#elif !defined(_TEA5767) && !defined(_RDA580X) && !defined(_TUX032) &&  defined(_LM7001)
	tuner.ic = TUNER_LM7001;
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
	case TUNER_RDA5807_DF:
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
	default:
		break;
	}

	tunerSetFreq();

	return;
}

void tunerSetFreq()
{
	if (tuner.freq < tuner.fMin)
		tuner.freq = tuner.fMin;
	else if (tuner.freq > tuner.fMax)
		tuner.freq = tuner.fMax;

	switch (tuner.ic) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		tea5767SetFreq();
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
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
	default:
		break;
	}

#ifdef _RDS
	rdsDisable(); // Clear RDS buffer
#endif

	return;
}

void tunerChangeFreq(int8_t mult)
{
	if ((tuner.freq > FM_BAND_DIV_FREQ) || (mult > 0 && tuner.freq == FM_BAND_DIV_FREQ))
		tuner.freq += tuner.step2 * mult;
	else
		tuner.freq += tuner.step1 * mult;

	tunerSetFreq();

	return;
}

void tunerReadStatus(void)
{
	switch (tuner.ic) {
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

void tunerSetMono(uint8_t value)
{
	switch (tuner.ic) {
	case TUNER_TEA5767:
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		tuner.mono = value;
		tunerSetFreq();
		break;
	default:
		tuner.mono = 0;
		break;
	}

	return;
}

void tunerSetRDS(uint8_t value)
{
	switch (tuner.ic) {
	case TUNER_RDA5807:
	case TUNER_RDA5807_DF:
		tuner.rds = value;
		tunerSetFreq();
		break;
	default:
		tuner.rds = 0;
		break;
	}
}

uint8_t tunerStereo(void)
{
	uint8_t ret = tuner.mono;

	if (ret)
		return 0;

	switch (tuner.ic) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		ret = TEA5767_BUF_STEREO(bufFM);
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		ret = RDA5807_BUF_STEREO(bufFM);
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

	switch (tuner.ic) {
#ifdef _TEA5767
	case TUNER_TEA5767:
		ret = (bufFM[3] & TEA5767_LEV_MASK) >> 4;
		break;
#endif
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		ret = (bufFM[2] & RDA580X_RSSI) >> 1;
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

/* Find station number (1..62) in EEPROM */
uint8_t tunerStationNum(void)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word((uint16_t*)EEPROM_STATIONS + i) == tuner.freq)
			return i + 1;

	return 0;
}

/* Find favourite station number (1..10) in EEPROM */
uint8_t tunerFavStationNum(void)
{
	uint8_t i;

	for (i = 0; i < FM_FAV_COUNT; i++)
		if (eeprom_read_word((uint16_t*)EEPROM_FAV_STATIONS + i) == tuner.freq)
			return i + 1;

	return 0;
}

/* Find nearest next/prev stored station */
void tunerNextStation(int8_t direction)
{
	uint8_t i;
	uint16_t freqCell;
	uint16_t freq = tuner.freq;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + i);
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

	return;
}

/* Load station by number */
void tunerLoadStation(uint8_t num)
{
	uint16_t freq = eeprom_read_word((uint16_t*)EEPROM_STATIONS + num);

	if (freq >= tuner.fMin && freq <= tuner.fMax) {
		tuner.freq = freq;
		tunerSetFreq();
	}

	return;
}

/* Load favourite station by number */
void tunerLoadFavStation(uint8_t num)
{
	uint16_t freq = eeprom_read_word((uint16_t*)EEPROM_FAV_STATIONS + num);

	if (freq >= tuner.fMin && freq <= tuner.fMax) {
		tuner.freq = freq;
		tunerSetFreq();
	}

	return;
}

/* Load favourite station by number */
void tunerStoreFavStation(uint8_t num)
{
	uint16_t freq = eeprom_read_word((uint16_t*)EEPROM_FAV_STATIONS + num);

	if (freq == tuner.freq)
		eeprom_update_word((uint16_t*)EEPROM_FAV_STATIONS + num, 0);
	else
		eeprom_update_word((uint16_t*)EEPROM_FAV_STATIONS + num, tuner.freq);

	return;
}

/* Save/delete station from eeprom */
void tunerStoreStation(void)
{
	uint8_t i, j;
	uint16_t freqCell;
	uint16_t freq;

	freq = tuner.freq;

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

void tunerSetVolume(int8_t value)
{
	tuner.volume = value;

	switch (tuner.ic) {
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		rda580xSetAudio();
		break;
#endif
	default:
		break;
	}

	return;
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
	case TUNER_RDA5807_DF:
		rda580xSetAudio();
		break;
#endif
#ifdef _TUX032
	case TUNER_TUX032:
		tux032SetMute();
		break;
#endif
	default:
		break;
	}

	return;
}

void tunerSetBass(uint8_t value)
{
	switch (tuner.ic) {
#ifdef _RDA580X
	case TUNER_RDA5807:
	case TUNER_RDA5802:
	case TUNER_RDA5807_DF:
		tuner.bass = value;
		rda580xSetAudio();
		break;
#endif
	default:
		tuner.bass = 0;
		break;
	}
}

void tunerPowerOn(void)
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

	tunerSetFreq();

	return;
}

void tunerPowerOff(void)
{
	eeprom_update_word((uint16_t*)EEPROM_FM_FREQ, tuner.freq);
	eeprom_update_byte((uint8_t*)EEPROM_FM_MONO, tuner.mono);

	switch (tuner.ic) {
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
