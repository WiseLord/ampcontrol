#include "tuner.h"

#include <avr/eeprom.h>
#include "eeprom.h"

uint8_t bufFM[5];

#if defined(TUX032)
uint16_t freqFM;
#endif

void tunerSearch(uint16_t freq, uint8_t direction)
{
#if defined(TEA5767)
	tea5767Search(freq, bufFM, direction);
#elif defined(TUX032)
	scanStoredFreq(freq, direction);
#endif
}

void tunerReadStatus()
{
#if defined(TEA5767)
	tea5767ReadStatus(bufFM);
#elif defined(TUX032)
	tux032ReadStatus(bufFM);
#endif
}

uint16_t tunerFreqAvail()
{
#if defined(TEA5767)
	return tea5767FreqAvail(bufFM);
#elif defined(TUX032)
	return freqFM;
#endif
}

uint8_t tunerReady()
{
#if defined(TEA5767)
	return TEA5767_BUF_READY(bufFM);
#elif defined(TUX032)
	return 1;
#endif
}

uint8_t tunerStereo()
{
#if defined(TEA5767)
	return TEA5767_BUF_STEREO(bufFM);
#elif defined(TUX032)
	return !TUX032_BUF_STEREO(bufFM);
#endif
}

uint8_t tunerLevel()
{
#if defined(TEA5767)
	return (bufFM[3] & TEA5767_LEV_MASK) >> 4;
#elif defined(TUX032)
	if (tunerStereo())
		return 13;
	else
		return 3;
#endif
}

void fineTune(uint16_t *freq)
{
#if defined(TEA5767)
	*freq = tea5767FreqAvail(bufFM);
#elif defined(TUX032)
	*freq = freqFM;
#endif
	if (*freq > FM_FREQ_MAX)
		*freq = FM_FREQ_MIN;
	if (*freq < FM_FREQ_MIN)
		*freq = FM_FREQ_MAX;

	tunerSetFreq(*freq);
}

/* Find station number (1..64) in EEPROM */
uint8_t stationNum(uint16_t freq)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word(eepromStations + i) == freq)
			return i + 1;

	return 0;
}

/* Find nearest next/prev stored station */
void scanStoredFreq(uint16_t freq, uint8_t direction)
{
	uint8_t i;
	uint16_t freqCell;
	uint16_t freqFound = freq;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell != 0xFFFF) {
			if (direction) {
				if (freqCell > freq) {
					freqFound = freqCell;
					break;
				}
			} else {
				if (freqCell < freq) {
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
void storeStation(uint16_t freq)
{
	uint8_t i, j;
	uint16_t freqCell;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell < freq)
			continue;
		if (freqCell == freq) {
			for (j = i; j < FM_COUNT; j++) {
				if (i == FM_COUNT - 1)
					freqCell = 0xFFFF;
				else
					freqCell = eeprom_read_word(eepromStations + j + 1);
				eeprom_write_word(eepromStations + j, freqCell);
			}
			break;
		} else {
			for (j = i; j < FM_COUNT; j++) {
				freqCell = eeprom_read_word(eepromStations + j);
				eeprom_write_word(eepromStations + j, freq);
				freq = freqCell;
			}
			break;
		}
	}

	return;
}
