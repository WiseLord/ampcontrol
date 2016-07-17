#include "tuner.h"

#include <avr/eeprom.h>
#include "../eeprom.h"

uint8_t bufFM[5];

uint8_t monoFM;
uint16_t freqFM;

void tunerInit()
{
	freqFM = eeprom_read_word((uint16_t*)EEPROM_FM_FREQ);
	monoFM = eeprom_read_byte((uint8_t*)EEPROM_FM_MONO);

#if defined(_TEA5767)
	tea5767Init();
#elif defined(_TUX032)
	tux032Init();
#elif defined(_LM7001)
	lm7001Init();
#elif defined(_RDA5807)
	rda5807Init();
#endif

	return;
}

void tunerSetFreq(uint16_t freq)
{
	if (freq > FM_FREQ_MAX)
		freq = FM_FREQ_MIN;
	if (freq < FM_FREQ_MIN)
		freq = FM_FREQ_MAX;

#if defined(_TEA5767)
	tea5767SetFreq(freq, monoFM);
#elif defined(_TUX032)
	tux032SetFreq(freq);
#elif defined(_LM7001)
	lm7001SetFreq(freq);
#elif defined(_RDA5807)
	rda5807SetFreq(freq, monoFM);
#endif
	freqFM = freq;

	return;
}

void tunerReadStatus()
{
#if defined(_TEA5767)
	tea5767ReadStatus(bufFM);
#elif defined(_TUX032)
	tux032ReadStatus(bufFM);
#elif defined(_RDA5807)
	rda5807ReadStatus(bufFM);
#endif

	return;
}

uint16_t tunerGetFreq()
{
	return freqFM;
}

void tunerSwitchMono()
{
#if defined(_TEA5767) || defined(_RDA5807)
	monoFM = !monoFM;
	tunerSetFreq(tunerGetFreq());
#endif
	return;
}

uint8_t tunerStereo()
{
#if defined(_TEA5767)
	return TEA5767_BUF_STEREO(bufFM) && !monoFM;
#elif defined(_TUX032)
	return !TUX032_BUF_STEREO(bufFM);
#elif defined(_LM7001)
	return 1;
#elif defined(_RDA5807)
	return RDA5807_BUF_STEREO(bufFM) && !monoFM;
#endif
}

uint8_t tunerLevel()
{
#if defined(_TEA5767)
	return (bufFM[3] & TEA5767_LEV_MASK) >> 4;
#elif defined(_TUX032)
	if (tunerStereo())
		return 13;
	else
		return 3;
#elif defined(_LM7001)
	return 13;
#elif defined(_RDA5807)
	uint8_t rawLevel = (bufFM[2] & RDA5807_RSSI) >> 1;
	if (rawLevel < 24)
		return 0;
	else
		return (rawLevel - 24) >> 1;
#endif
}

/* Find station number (1..64) in EEPROM */
uint8_t tunerStationNum(uint16_t freq)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word((uint16_t*)EEPROM_STATIONS + i) == freq)
			return i + 1;

	return 0;
}

/* Find nearest next/prev stored station */
void tunerNextStation(uint8_t direction)
{
	uint8_t i;
	uint16_t freqCell;
	uint16_t freqFound = freqFM;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + i);
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
void tunerLoadStation(uint8_t num)
{
	uint16_t freqCell = eeprom_read_word((uint16_t*)EEPROM_STATIONS + num);

	if (freqCell != 0xFFFF)
		tunerSetFreq(freqCell);

	return;
}

/* Save/delete station from eeprom */
void tunerStoreStation(void)
{
	uint8_t i, j;
	uint16_t freqCell;
	uint16_t freq;

	freq = freqFM;

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

void tunerPowerOn(void)
{
	tunerSetFreq(freqFM);

	return;
}

void tunerPowerOff(void)
{
	eeprom_update_word((uint16_t*)EEPROM_FM_FREQ, freqFM);
	eeprom_update_byte((uint8_t*)EEPROM_FM_MONO, monoFM);

#if defined(_TUX032)
	tux032GoStby();
#endif

	return;
}
