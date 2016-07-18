#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "tuner/tuner.h"
#include "adc.h"
#include "rtc.h"

static int8_t brStby;							/* Brightness in standby mode */
static int8_t brWork;							/* Brightness in working mode */
static char strbuf[STR_BUFSIZE + 1];			/* String buffer */

uint8_t *txtLabels[LABEL_END];				/* Array with text label pointers */

static const uint8_t barSymbols[] PROGMEM = {
	0x00, 0x10, 0x14, 0x15,
};

static void writeStringEeprom(const uint8_t *string)
{
	uint8_t i = 0;

	for (i = 0; i < STR_BUFSIZE; i++)
		strbuf[i] = eeprom_read_byte(&string[i]);

	ks0066WriteString(strbuf);

	return;
}

static void lcdGenLevels(void)
{
	ks0066WriteCommand(KS0066_SET_CGRAM);

	uint8_t i, j;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			if (j + i >= 7) {
				ks0066WriteData(0xFF);
			} else {
				ks0066WriteData(0x00);
			}
		}
	}

	return;
}

static void lcdGenBar(void)
{
	ks0066WriteCommand(KS0066_SET_CGRAM);

	uint8_t i, j;
	uint8_t sym;

	for (j = 0; j < 4; j++) {
		for (i = 0; i < 8; i++) {
			sym = j;
			if (i == 3)
				sym = 3;
			if (i == 7)
				sym = 0;
			ks0066WriteData(pgm_read_byte(&barSymbols[sym]));
		}
	}

	return;
}

static void writeNum(int8_t value, uint8_t width, uint8_t lead)
{
	uint8_t sign = lead;
	int8_t pos;

	if (value < 0) {
		sign = '-';
		value = -value;
	}

	for (pos = 0; pos < width; pos++)
		strbuf[pos] = lead;
	strbuf[width] = '\0';
	pos = width - 1;

	while (value > 0 || pos == width - 1) {
		strbuf[pos] = value % 10 + 0x30;
		pos--;
		value /= 10;
	}
	if (pos >= 0)
		strbuf[pos] = sign;

	ks0066WriteString(strbuf);

	return;
}

static void showBar(int16_t min, int16_t max, int16_t value)
{
	uint8_t i;

	lcdGenBar();
	ks0066SetXY(0, 1);

	value = (value - min) * 48 / (max - min);
	for (i = 0; i < 16; i++) {
		if (value / 3 > i) {
			ks0066WriteData(0x03);
		} else {
			if (value / 3 < i) {
				ks0066WriteData(0x00);
			} else {
				ks0066WriteData(value % 3);
			}
		}
	}

	return;
}

void showRC5Info(uint16_t rc5Buf)
{
	ks0066SetXY(0, 0);
	ks0066WriteString("RC=");
	writeNum((rc5Buf >> 6) & 0x1F, 2, ' ');
	ks0066SetXY(0, 1);
	ks0066WriteString("CM=");
	writeNum(rc5Buf & 0x3F, 2, ' ');

	return;
}

void showRadio(void)
{
	uint8_t num = tunerStationNum();

	/* Frequency value */
	ks0066SetXY(0, 0);
	ks0066WriteString("FM ");
	writeNum(tuner.freq / 100, 3, ' ');
	ks0066WriteData('.');
	writeNum(tuner.freq / 10 % 10, 1, ' ');

	/* Stereo indicator */
	ks0066SetXY(9, 0);
	if (tunerStereo())
		ks0066WriteData('S');
	else
		ks0066WriteData('M');

	/* Signal level */
	// Temporary disabled

	/* Station number */
	ks0066SetXY(13, 0);
	if (num) {
		writeNum(num, 3, ' ');
	} else {
		ks0066WriteString(" --");
	}

	/* Frequency scale */
	showBar(0, (tuner.fMax - tuner.fMin) >> 4, (tuner.freq - tuner.fMin) >> 4);

	return;
}

void showBoolParam(uint8_t value, uint8_t labelIndex)
{
	ks0066SetXY(0, 0);
	writeStringEeprom(txtLabels[labelIndex]);
	ks0066SetXY(1, 1);
	if (value)
		writeStringEeprom(txtLabels[LABEL_ON]);
	else
		writeStringEeprom(txtLabels[LABEL_OFF]);

	return;
}

/* Show brightness control */
void showBrWork(void)
{
	ks0066SetXY(0, 0);
	writeStringEeprom(txtLabels[LABEL_BR_WORK]);

	ks0066SetXY(13, 0);
	writeNum(brWork, 3, ' ');

	showBar(DISP_MIN_BR, DISP_MAX_BR, brWork);

	return;
}

void changeBrWork(int8_t diff)
{
	brWork += diff;
	if (brWork > DISP_MAX_BR)
		brWork = DISP_MAX_BR;
	if (brWork < DISP_MIN_BR)
		brWork = DISP_MIN_BR;
	setWorkBrightness();

	return;
}

/* Show audio parameter */
void showSndParam(sndParam *param)
{
	ks0066SetXY(0, 0);
	writeStringEeprom(param->label);

	//	(((int16_t)(param->value) * param->step + 4) >> 3);
	ks0066SetXY(11, 0);
	writeNum(param->value, 3, ' ');

	ks0066SetXY(14, 0);
	writeStringEeprom(txtLabels[LABEL_DB]);

	showBar(param->min, param->max, param->value);

	return;
}

static void drawTm(uint8_t tm)
{
	if (tm == rtc.etm && getRtcTimer() < RTC_POLL_TIME / 2)
		ks0066WriteString("  ");
	else
		writeNum(*((int8_t*)&rtc + tm), 2, '0');

	return;
}

void showTime(void)
{
	ks0066SetXY(0, 0);

	drawTm(RTC_HOUR);
	ks0066WriteData(':');
	drawTm(RTC_MIN);
	ks0066WriteData(':');
	drawTm(RTC_SEC);


	ks0066SetXY(11, 0);
	drawTm(RTC_DATE);
	ks0066WriteData('.');
	drawTm(RTC_MONTH);

	ks0066SetXY(12, 1);
	writeNum(20, 2, '0');
	drawTm(RTC_YEAR);

	ks0066SetXY(0, 1);

	writeStringEeprom(txtLabels[LABEL_SUNDAY + (rtc.wday - 1) % 7]);

	return;
}

void showSpectrum(uint8_t *buf)
{
	uint8_t i;
	uint8_t val;

	lcdGenLevels();

	for (i = 0; i < 16; i++) {
		val = buf[i];
		if (val > 15)
			val = 15;
		ks0066SetXY(i, 0);
		if (val < 8)
			ks0066WriteData(0x20);
		else
			ks0066WriteData(val - 8);
		ks0066SetXY(i, 1);
		if (val < 8)
			ks0066WriteData(val);
		else
			ks0066WriteData(0xFF);
	}

	return;
}

void setWorkBrightness(void)
{
	setDispBr(brWork);

	return;
}

void setStbyBrightness(void)
{
	setDispBr(brStby);
	eeprom_update_byte((uint8_t*)EEPROM_BR_WORK, brWork);

	return;
}

void loadDispSndParams(void)
{
	uint8_t i;
	uint8_t *addr;

	brStby = eeprom_read_byte((uint8_t*)EEPROM_BR_STBY);
	brWork = eeprom_read_byte((uint8_t*)EEPROM_BR_WORK);

	/* Load text labels from EEPROM */
	addr = (uint8_t*)EEPROM_LABELS_ADDR;
	i = 0;

	while (i < LABEL_END && addr < (uint8_t*)EEPROM_SIZE) {
		if (eeprom_read_byte(addr) != '\0') {
			txtLabels[i] = addr;
			addr++;
			i++;
			while (eeprom_read_byte(addr) != '\0' &&
				   addr < (uint8_t*)EEPROM_SIZE) {
				addr++;
			}
		} else {
			addr++;
		}
	}

	// Init audio labels and parameters
	loadAudioParams(txtLabels);

	return;
}
