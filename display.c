#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "tuner.h"

static uint8_t backlight;
static uint8_t strbuf[STR_BUFSIZE + 1] = "                ";	/* String buffer */
static uint8_t userSybmols = LCD_LEVELS;

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

void displayInit()
{
	ks0066Init();
	lcdGenLevels();

#if defined(KS0066)
	KS0066_BCKL_DDR |= KS0066_BCKL;
#endif

	return;
}

static uint8_t *mkNumString(int16_t value, uint8_t width, uint8_t lead)
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
	strbuf[1] = value % 16 + 30;
	strbuf[0] = value / 16 % 16 + 30;

	return strbuf;
}

static void showBar(int16_t min, int16_t max, int16_t value)
{
	uint8_t i;

	if (userSybmols != LCD_BAR) {
		lcdGenBar();
		userSybmols = LCD_BAR;
	}

	ks0066SetXY(0, 1);

	value = (int16_t)48 * (value - min) / (max - min);
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

static void showParValue(int8_t value)
{
	ks0066SetXY(11, 0);
	ks0066WriteString(mkNumString(value, 3, ' '));

	return;
}

static void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels)
{
	ks0066SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0066SetXY(14, 0);
	writeStringEeprom(txtLabels[LABEL_DB]);

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

void showRadio(uint8_t num)
{
	uint16_t freq = tunerGetFreq();

	uint8_t lev;

	/* Frequency value */
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"FM ");
	ks0066WriteString(mkNumString(freq / 100, 3, ' '));
	ks0066WriteData('.');
	ks0066WriteString(mkNumString(freq / 10 % 10, 1, ' '));

	/* Stereo indicator */
	ks0066SetXY(9, 0);
	if (tunerStereo())
		ks0066WriteString((uint8_t*)"S");
	else
		ks0066WriteString((uint8_t*)" ");

	/* Signal level */
	ks0066SetXY(11, 0);
	lev = tunerLevel() * 13 / 32;
	if (lev < 3) {
		ks0066WriteData(lev);
		ks0066WriteData(0x00);
	} else {
		ks0066WriteData(0x03);
		ks0066WriteData(lev - 3);
	}

	/* Frequency scale */
	showBar(FM_FREQ_MIN>>4, FM_FREQ_MAX>>4, freq>>4);

	/* Station number */
	ks0066SetXY(14, 0);
	if (num) {
		ks0066WriteString(mkNumString(num, 2, ' '));
	} else {
		ks0066WriteString((uint8_t*)"--");
	}

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

/* Show audio parameter */
void showSndParam(sndParam *param, uint8_t **txtLabels)
{
	showBar(param->min, param->max, param->value);
	showParValue(((int16_t)(param->value) * param->step + 4) >> 3);
	showParLabel(param->label, txtLabels);

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

	writeStringEeprom(txtLabels[LABEL_MONDAY + getTime(WEEK) % 7]);

	if (getEtm() == NOEDIT) {
		ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	} else {
		switch (getEtm()) {
		case HOUR:
			ks0066SetXY(1, 0);
			break;
		case MIN:
			ks0066SetXY(4, 0);
			break;
		case SEC:
			ks0066SetXY(7, 0);
			break;
		case DAY:
			ks0066SetXY(12, 0);
			break;
		case MONTH:
			ks0066SetXY(15, 0);
			break;
		case YEAR:
			ks0066SetXY(15, 1);
			break;
		default:
			break;
		}
		ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON | KS0066_CUR_BLINK_ON);
	}

	return;
}

void drawSpectrum(uint8_t *buf)
{
	uint8_t i;

	if (userSybmols != LCD_LEVELS) {
		lcdGenLevels();
		userSybmols = LCD_LEVELS;
	}

	for (i = 0; i < 16; i++) {

		ks0066SetXY(i, 0);
		if (buf[i] < 8)
			ks0066WriteData(0x20);
		else
			ks0066WriteData(buf[i] - 8);
		ks0066SetXY(i, 1);
		if (buf[i] < 8)
			ks0066WriteData(buf[i]);
		else
			ks0066WriteData(0xFF);
	}

	return;
}

void loadDispParams(void)
{
//	backlight = eeprom_read_byte(eepromBCKL);
	ks0066Backlight(backlight);

	return;
}

void saveDisplayParams(void)
{
//	eeprom_update_byte(eepromBCKL, backlight);

	return;
}

/* Change backlight status */
void switchBacklight(void)
{
	backlight = !backlight;
	ks0066Backlight(backlight);

	return;
}
