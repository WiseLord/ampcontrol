#include "tuner.h"

#include <avr/eeprom.h>
#include "../eeprom.h"

uint8_t *bufFM;
static tunerIC _tuner;

static uint16_t freqFM;
static uint8_t monoFM;
static uint8_t stepFM;

void tunerInit()
{
	_tuner = TUNER_RDA5807;
	uint8_t ctrl = eeprom_read_byte(eepromFMCtrl);

	switch (_tuner) {
	case TUNER_TEA5767:
		tea5767Init(ctrl);
		break;
	case TUNER_RDA5807:
		rda5807Init();
		break;
	case TUNER_TUX032:
		tux032Init();
		break;
	default:
		break;
	}

	return;
}

void tunerSetFreq(uint16_t freq)
{
	if (freq > FM_FREQ_MAX)
		freq = FM_FREQ_MAX;
	if (freq < FM_FREQ_MIN)
		freq = FM_FREQ_MIN;

	switch (_tuner) {
	case TUNER_TEA5767:
		tea5767SetFreq(freq, monoFM);
		break;
	case TUNER_RDA5807:
		rda5807SetFreq(freq, monoFM);
		break;
	case TUNER_TUX032:
		tux032SetFreq(freq);
		break;
	default:
		break;
	}

	freqFM = freq;

	return;
}

uint16_t tunerGetFreq()
{
	return freqFM;
}

void tunerIncFreq(uint8_t mult)
{
	tunerSetFreq(freqFM + FM_STEP * mult);

	return;
}

void tunerDecFreq(uint8_t mult)
{
	tunerSetFreq(freqFM - FM_STEP * mult);

	return;
}

void tunerReadStatus()
{
	switch (_tuner) {
	case TUNER_TEA5767:
		bufFM = tea5767ReadStatus();
		break;
	case TUNER_RDA5807:
		bufFM = rda5807ReadStatus();
		break;
	case TUNER_TUX032:
		bufFM = tux032ReadStatus();
		break;
	default:
		break;
	}

	return;
}

void tunerSwitchMono()
{
	monoFM = !monoFM;

	if (_tuner == TUNER_TEA5767 || _tuner == TUNER_RDA5807)
		tunerSetFreq(tunerGetFreq());

	return;
}

uint8_t tunerStereo()
{
	uint8_t ret = 1;

	switch (_tuner) {
	case TUNER_TEA5767:
		ret = TEA5767_BUF_STEREO(bufFM) && !monoFM;
		break;
	case TUNER_RDA5807:
		ret = RDA5807_BUF_STEREO(bufFM) && !monoFM;
		break;
	case TUNER_TUX032:
		ret = !TUX032_BUF_STEREO(bufFM);
		break;
	default:
		break;
	}

	return ret;
}

uint8_t tunerLevel()
{
	uint8_t ret = 0;
	uint8_t rawLevel;

	switch (_tuner) {
	case TUNER_TEA5767:
		ret = (bufFM[3] & TEA5767_LEV_MASK) >> 4;
		break;
	case TUNER_RDA5807:
		rawLevel = (bufFM[2] & RDA5807_RSSI) >> 1;
		if (rawLevel < 24)
			ret = 0;
		else
			ret = (rawLevel - 24) >> 1;
		break;
	case TUNER_TUX032:
		if (tunerStereo())
			ret = 13;
		else
			ret = 3;
		break;
	default:
		break;
	}

	return ret;
}

/* Find station number (1..64) in EEPROM */
uint8_t stationNum(void)
{
	uint8_t i;

	uint16_t freq = tunerGetFreq();

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word(eepromStations + i) == freq)
			return i + 1;

	return 0;
}

/* Find nearest next/prev stored station */
void scanStoredFreq(uint8_t direction)
{
	uint8_t i;
	uint16_t freqCell;
	uint16_t freqFound = freqFM;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell != 0xFFFF) {
			if (direction) {
				if (freqCell > freqFM) {
					freqFound = freqCell;
					break;
				}
			} else {
				if (freqCell < freqFM) {
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
void loadStation(uint8_t num)
{
	uint16_t freqCell = eeprom_read_word(eepromStations + num);

	if (freqCell != 0xFFFF)
		tunerSetFreq(freqCell);

	return;
}

/* Save/delete station from eeprom */
void storeStation(void)
{
	uint8_t i, j;
	uint16_t freqCell;
	uint16_t freq;

	freq = freqFM;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell < freq)
			continue;
		if (freqCell == freq) {
			for (j = i; j < FM_COUNT; j++) {
				if (j == FM_COUNT - 1)
					freqCell = 0xFFFF;
				else
					freqCell = eeprom_read_word(eepromStations + j + 1);
				eeprom_update_word(eepromStations + j, freqCell);
			}
			break;
		} else {
			for (j = i; j < FM_COUNT; j++) {
				freqCell = eeprom_read_word(eepromStations + j);
				eeprom_update_word(eepromStations + j, freq);
				freq = freqCell;
			}
			break;
		}
	}

	return;
}

void loadTunerParams(void)
{
	freqFM = eeprom_read_word(eepromFMFreq);
	monoFM = eeprom_read_byte(eepromFMMono);
	stepFM = eeprom_read_byte(eepromFMStep);

	return;
}

void setTunerParams(void)
{
	tunerSetFreq(freqFM);

	return;
}

void saveTunerParams(void)
{
	eeprom_update_word(eepromFMFreq, freqFM);
	eeprom_update_byte(eepromFMMono, monoFM);

	switch (_tuner) {
	case TUNER_TUX032:
		tux032GoStby();
		break;
	default:
		break;
	}

	return;
}
