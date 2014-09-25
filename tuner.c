#include "tuner.h"

#include <avr/eeprom.h>
#include "eeprom.h"

uint8_t bufFM[5];

uint8_t monoFM;
uint16_t freqFM;

void tunerInit()
{
#if defined(TEA5767)
	tea5767Init();
#elif defined(TUX032)
	tux032Init();
#elif defined(LM7001)
	lm7001Init();
#elif defined(RDA5807)
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

#if defined(TEA5767)
	tea5767SetFreq(freq, monoFM);
#elif defined(TUX032)
	tux032SetFreq(freq);
#elif defined(LM7001)
	lm7001SetFreq(freq);
#elif defined(RDA5807)
	rda5807SetFreq(freq, monoFM);
#endif
	freqFM = freq;

	return;
}

void tunerReadStatus()
{
#if defined(TEA5767)
	tea5767ReadStatus(bufFM);
#elif defined(TUX032)
	tux032ReadStatus(bufFM);
#elif defined(RDA5807)
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
#if defined(TEA5767) || defined(RDA5807)
	monoFM = !monoFM;
	tunerSetFreq(tunerGetFreq());
#endif
	return;
}

uint8_t tunerReady()
{
#if defined(TEA5767)
	return TEA5767_BUF_READY(bufFM);
#elif defined(TUX032) || defined (LM7001)
	return 1;
#elif defined(RDA5807)
	return RDA5807_BUF_READY(bufFM);
#endif
}

uint8_t tunerStereo()
{
#if defined(TEA5767)
	return TEA5767_BUF_STEREO(bufFM) && !monoFM;
#elif defined(TUX032)
	return !TUX032_BUF_STEREO(bufFM);
#elif defined(LM7001)
	return 1;
#elif defined(RDA5807)
	return RDA5807_BUF_STEREO(bufFM) && !monoFM;
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
#elif defined(LM7001)
	return 13;
#elif defined(RDA5807)
	return (bufFM[0] & RDA5807_RSSI) >> 4;
#endif
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

	tunerSetFreq(freqFM);

	return;
}

void saveTunerParams(void)
{
	eeprom_update_word(eepromFMFreq, freqFM);
	eeprom_update_byte(eepromFMMono, monoFM);

#if defined(TUX032)
	tux032GoStby();
#endif

	return;
}
