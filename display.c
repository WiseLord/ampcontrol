#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "tuner.h"
#include "adc.h"

static int8_t brStby;							/* Brightness in standby mode */
static int8_t brWork;							/* Brightness in working mode */
static uint8_t strbuf[STR_BUFSIZE + 1];			/* String buffer */

static const uint8_t timeCurPos[] PROGMEM = {
	7, 4, 1, 0, 12, 15, 79
};

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

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (i + j >= 7) {
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

	uint8_t i;
	uint8_t sym;

	for (i = 0; i < 8 * 4; i++) {
		sym = i / 8;
		if (i % 8 == 3)
			sym = 3;
		if (i % 8 == 7)
			sym = 0;
		ks0066WriteData(pgm_read_byte(&barSymbols[sym]));
	}

	return;
}

static uint8_t *mkNumString(int8_t value, uint8_t width, uint8_t lead)
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

	return strbuf;
}

static uint8_t *mkHexString(uint8_t value)
{
	strbuf[2] = '\0';
	strbuf[1] = value % 16 + 0x30;
	strbuf[0] = value / 16 % 16 + 0x30;

	return strbuf;
}

static void showBar(int8_t min, int8_t max, int8_t value)
{
	uint8_t i;

	lcdGenBar();
	ks0066SetXY(0, 1);

	value = (int16_t)(value - min) * 48 / (max - min);
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

static void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels)
{
	ks0066SetXY(0, 0);
	writeStringEeprom(parLabel);

	return;
}

void showRC5Info(uint16_t rc5Buf)
{
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"RC=");
	ks0066WriteString(mkHexString((rc5Buf >> 6) & 0x1F));
	ks0066SetXY(0, 1);
	ks0066WriteString((uint8_t*)"CM=");
	ks0066WriteString(mkHexString(rc5Buf & 0x3F));

	return;
}

void showRadio(void)
{
	uint16_t freq = tunerGetFreq();
	uint8_t num = stationNum(freq);

	uint8_t lev;

	/* Frequency value */
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"FM ");
	ks0066WriteString(mkNumString(freq / 100, 3, ' '));
	ks0066WriteData('.');
	ks0066WriteString(mkNumString(freq / 10 % 10, 1, ' '));

	/* Stereo indicator */
	if (tunerStereo())
		ks0066WriteString((uint8_t*)" S ");
	else
		ks0066WriteString((uint8_t*)"   ");

	/* Signal level */
	lev = tunerLevel() * 13 / 32;
	if (lev < 3) {
		ks0066WriteData(lev);
		ks0066WriteData(0x00);
	} else {
		ks0066WriteData(0x03);
		ks0066WriteData(lev - 3);
	}

	/* Station number */
	if (num) {
		ks0066WriteString(mkNumString(num, 3, ' '));
	} else {
		ks0066WriteString((uint8_t*)" --");
	}

	/* Frequency scale */
	showBar(0, (FM_FREQ_MAX - FM_FREQ_MIN)>>5, (freq - FM_FREQ_MIN)>>5);

	return;
}

void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels)
{
	ks0066SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0066SetXY(1, 1);
	if (value)
		writeStringEeprom(txtLabels[LABEL_ON]);
	else
		writeStringEeprom(txtLabels[LABEL_OFF]);

	return;
}

/* Show brightness control */
void showBrWork(uint8_t **txtLabels)
{
	showBar(DISP_MIN_BR, DISP_MAX_BR, brWork);
	ks0066SetXY(13, 0);
//	showParValue(brWork);
	ks0066WriteString(mkNumString(brWork, 3, ' '));
	showParLabel(txtLabels[LABEL_BR_WORK], txtLabels);

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
void showSndParam(sndParam *param, uint8_t **txtLabels)
{
	showBar(param->min, param->max, param->value);
	ks0066SetXY(11, 0);
//	showParValue(((int16_t)(param->value) * param->step + 4) >> 3);
	ks0066WriteString(mkNumString(param->value, 3, ' '));
	showParLabel(param->label, txtLabels);
	ks0066SetXY(14, 0);
	writeStringEeprom(txtLabels[LABEL_DB]);

	return;
}

static void drawTm(timeMode tm)
{
	ks0066WriteString(mkNumString(getTime(tm), 2, '0'));

	return;
}

void showTime(uint8_t **txtLabels)
{
	readTime();
	ks0066SetXY(0, 0);

	drawTm(HOUR);
	ks0066WriteData(':');
	drawTm(MIN);
	ks0066WriteData(':');
	drawTm(SEC);

	ks0066SetXY(11, 0);
	drawTm(DAY);
	ks0066WriteData('.');
	drawTm(MONTH);

	ks0066SetXY(12, 1);
	ks0066WriteString((uint8_t*)"20");
	drawTm(YEAR);

	ks0066SetXY(0, 1);

	writeStringEeprom(txtLabels[LABEL_SUNDAY + getTime(WEEK) % 7]);

	if (getEtm() == NOEDIT) {
		ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	} else {
		ks0066SetXY(pgm_read_byte(&timeCurPos[getEtm()]), 0);
		ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON | KS0066_CUR_BLINK_ON);
	}

	return;
}

void drawSpectrum(uint8_t *buf)
{
	uint8_t i;
	uint8_t val;

	lcdGenLevels();

	for (i = 0; i < 16; i++) {
		val = buf[i];
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

#if defined(PCF8574)
	if (brWork == GD_MAX_BRIGTHNESS)
		pcf8574IntBacklight(BACKLIGHT_ON);
	else
		pcf8574IntBacklight(BACKLIGHT_OFF);
#endif

	return;
}

void setStbyBrightness(void)
{
	setDispBr(brStby);

#if defined(PCF8574)
	pcf8574IntBacklight(BACKLIGHT_OFF);
#endif

	return;
}

void loadDispParams(void)
{
	brStby = eeprom_read_byte(eepromBrStby);
	brWork = eeprom_read_byte(eepromBrWork);

	return;
}

void saveDisplayParams(void)
{
	eeprom_update_byte(eepromBrStby, brStby);
	eeprom_update_byte(eepromBrWork, brWork);

	return;
}
