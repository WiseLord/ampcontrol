#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "tuner.h"

uint8_t backlight;										/* Backlight */
uint8_t strbuf[STR_BUFSIZE + 1] = "                ";	/* String buffer */

uint8_t defDisplay = MODE_SPECTRUM;						/* Default display */

static uint8_t userSybmols = LCD_LEVELS;

uint8_t getDefDisplay()
{
	return defDisplay;
}

void setDefDisplay(uint8_t value)
{
	defDisplay = value;

	return;
}

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
	uint8_t pos[] = {0x00, 0x10, 0x14, 0x15, 0x05, 0x01};

	for (i = 0; i < 48; i++) {
		if ((i & 0x07) == 0x03) {
			ks0066WriteData(0x15);
		} else if ((i & 0x07) == 0x07) {
			ks0066WriteData(0x00);
		} else {
			ks0066WriteData(pos[i>>3]);
		}
	}

	return;
}

void displayInit()
{
	ks0066Init();
	lcdGenLevels();

	KS0066_BCKL_DDR |= KS0066_BCKL;

	return;
}

void clearDisplay()
{
	ks0066Clear();

	return;
}

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix)
{
	uint8_t numdiv;
	uint8_t sign = lead;
	int8_t i;

	if (number < 0) {
		sign = '-';
		number = -number;
	}

	for (i = 0; i < width; i++)
		strbuf[i] = lead;
	strbuf[width] = '\0';
	i = width - 1;
	while (number > 0 || i == width - 1) {
		numdiv = number % radix;
		strbuf[i] = numdiv + 0x30;
		if (numdiv >= 10)
			strbuf[i] += 7;
		i--;
		number /= radix;
	}
	if (i >= 0)
		strbuf[i] = sign;

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
	ks0066WriteString(mkNumString(value, 3, ' ', 10));

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
	ks0066WriteString((uint8_t*)",RC=");
	ks0066WriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	ks0066SetXY(0, 1);
	ks0066WriteString((uint8_t*)",CM=");
	ks0066WriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));

	return;
}

void showRadio(uint8_t num)
{
	uint16_t freq = tunerGetFreq();

	uint8_t lev;

	/* Frequency value */
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"FM ");
	ks0066WriteString(mkNumString(freq / 100, 3, ' ', 10));
	ks0066WriteData('.');
	ks0066WriteString(mkNumString(freq / 10 % 10, 1, ' ', 10));

	/* Stereo indicator */
	ks0066SetXY(9, 0);
	if (tunerStereo())
		ks0066WriteString((uint8_t*)"S");
	else
		ks0066WriteString((uint8_t*)" ");

	/* Signal level */
	ks0066SetXY(11, 0);
	lev = tunerLevel() * 2 / 5;
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
		ks0066WriteString(mkNumString(num, 2, ' ', 10));
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
	ks0066WriteString(mkNumString(getTime(tm), 2, '0', 10));

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
	uint8_t lcdBuf[16];

	if (userSybmols != LCD_LEVELS) {
		lcdGenLevels();
		userSybmols = LCD_LEVELS;
	}

	for (i = 0; i < 16; i++) {
		lcdBuf[i] = buf[2 * i] + buf[2 * i + 1];
		lcdBuf[i] = buf[32 + 2 * i] + buf[32 + 2 * i + 1];
		lcdBuf[i] >>= 2;
	}

	uint8_t data;
	ks0066SetXY(0, 0);
	for (i = 0; i < 16; i++) {
		data = ' ';
		if (lcdBuf[i] >= 8)
			data = lcdBuf[i] - 8;
		ks0066WriteData(data);
	}
	ks0066SetXY(0, 1);
	for (i = 0; i < 16; i++) {
		data = 0xFF;
		if (lcdBuf[i] < 8)
			data = lcdBuf[i];
		ks0066WriteData(data);
	}

	return;
}

void loadDispParams(void)
{
	backlight = eeprom_read_byte(eepromBCKL);
	setBacklight(backlight);
	defDisplay = eeprom_read_byte(eepromDisplay);

	return;
}

void saveDisplayParams(void)
{
	eeprom_update_byte(eepromBCKL, backlight);
	eeprom_update_byte(eepromDisplay, defDisplay);

	return;
}

/* Turn on/off backlight */
void setBacklight(int8_t backlight)
{
#if defined(PCF8574)
	ks0066Backlight(backlight);
#else
	if (backlight)
		KS0066_BCKL_PORT |= KS0066_BCKL;
	else
		KS0066_BCKL_PORT &= ~KS0066_BCKL;
#endif
	return;
}

/* Change backlight status */
void switchBacklight(void)
{
	backlight = !backlight;
	setBacklight(backlight);

	return;
}
