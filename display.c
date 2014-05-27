#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"

#include "tuner.h"

uint8_t backlight;										/* Backlight */
uint8_t spMode;											/* Spectrum mode */
uint8_t strbuf[STR_BUFSIZE + 1] = "                ";	/* String buffer */

uint8_t defDisplay = MODE_SPECTRUM;						/* Default display */

uint8_t getDefDisplay()
{
	return defDisplay;
}

void setDefDisplay(uint8_t value)
{
	defDisplay = value;

	return;
}

#if defined(KS0066)
static uint8_t userSybmols = LCD_LEVELS;
#endif

static void writeStringEeprom(const uint8_t *string)
{
	uint8_t i = 0;

	for (i = 0; i < STR_BUFSIZE; i++)
		strbuf[i] = eeprom_read_byte(&string[i]);

#if defined(KS0108)
	ks0108WriteString(strbuf);
#elif defined(KS0066)
	ks0066WriteString(strbuf);
#endif

	return;
}

#if defined(KS0066)
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
	/* Stereo indicator */
	ks0066WriteData(0b00000000);
	ks0066WriteData(0b00011011);
	ks0066WriteData(0b00010101);
	ks0066WriteData(0b00010101);
	ks0066WriteData(0b00010101);
	ks0066WriteData(0b00011011);
	ks0066WriteData(0b00000000);
	ks0066WriteData(0b00000000);
}
#endif

void displayInit()
{
#if defined(KS0108)
	ks0108Init();
	ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(KS0066)
	ks0066Init();
	lcdGenLevels();
#endif
}

void clearDisplay()
{
#if defined(KS0108)
	ks0108Fill(0x00);
#elif defined(KS0066)
	ks0066Clear();
#endif
}

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix)
{
	uint8_t numdiv;
	uint8_t sign = lead;
	if (number < 0) {
		sign = '-';
		number = -number;
	}
	int8_t i;
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
#if defined(KS0108)
	uint8_t data;
	uint8_t i, j;

	if (min + max) {
		value = (int16_t)85 * (value - min) / (max - min);
	} else {
		value = (int16_t)42 * value / max;
	}
	for (j = 5; j <= 6; j++) {
		ks0108SetXY(0, j);
		for (i = 0; i < 85; i++) {
			if (((min + max) && (value <= i)) || (!(min + max) &&
				(((value > 0) && ((i < 42) || (value + 42 < i))) ||
				((value <= 0) && ((i > 42) || (value + 42 > i)))))) {
				if (j == 5) {
					data = 0x80;
				} else {
					data = 0x01;
				}
			} else {
				data = 0xFF;
			}
			if (i & 0x01) {
				data = 0x00;
			}
			ks0108WriteData(data);
		}
	}
#elif defined(KS0066)
	uint8_t i;

	if (userSybmols != LCD_BAR) {
		lcdGenBar();
		userSybmols = LCD_BAR;
	}

	ks0066SetXY(0, 1);

	if (min + max) {
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
	} else {
		value = (int16_t)23 * value / max;
		if (value >= 0) {
			value++;
			for (i = 0; i < 7; i++) {
				ks0066WriteData(0x00);
			}
			ks0066WriteData(0x05);
			for (i = 0; i < 8; i++) {
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
		} else {
			value += 23;
			for (i = 0; i < 8; i++) {
				if (value / 3 > i) {
					ks0066WriteData(0x00);
				} else {
					if (value / 3 < i) {
						ks0066WriteData(0x03);
					} else {
						ks0066WriteData(value % 3 + 3);
					}
				}
			}
			ks0066WriteData(0x01);
			for (i = 0; i < 7; i++) {
				ks0066WriteData(0x00);
			}
		}
	}
#endif
}

static void showParValue(int8_t value)
{
#if defined(KS0108)
	ks0108LoadFont(font_ks0066_ru_24, 1);
	ks0108SetXY(93, 4);
	ks0108WriteString(mkNumString(value, 3, ' ', 10));
	ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(KS0066)
	ks0066SetXY(11, 0);
	ks0066WriteString(mkNumString(value, 3, ' ', 10));
#endif
}

static void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels)
{
#if defined(KS0108)
	ks0108LoadFont(font_ks0066_ru_24, 1);
	ks0108SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0108LoadFont(font_ks0066_ru_08, 1);
	ks0108SetXY(116, 7);
	writeStringEeprom(txtLabels[LABEL_DB]);
#elif defined (KS0066)
	ks0066SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0066SetXY(14, 0);
	writeStringEeprom(txtLabels[LABEL_DB]);
#endif
}

void showRC5Info(uint16_t rc5Buf)
{
#if defined(KS0108)
	ks0108LoadFont(font_ks0066_ru_08, 1);
	ks0108SetXY(0, 0);
	ks0108WriteString((uint8_t*)"RC5:");
	ks0108SetXY(5, 1);
	ks0108WriteString((uint8_t*)"Raw = ");
	ks0108WriteString(mkNumString(rc5Buf, 14, '0', 2));
	ks0108SetXY(5, 2);
	ks0108WriteString((uint8_t*)"Tog = ");
	ks0108WriteString(mkNumString(((rc5Buf & 0x0800) > 0), 1, '0', 16));
	ks0108SetXY(5, 3);
	ks0108WriteString((uint8_t*)"Adr = ");
	ks0108WriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	ks0108SetXY(5, 4);
	ks0108WriteString((uint8_t*)"Cmd = ");
	ks0108WriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));
	ks0108SetXY(0, 6);
	ks0108WriteString((uint8_t*)"Buttons:");
	ks0108SetXY(5, 7);
	ks0108WriteString(mkNumString(BTN_PIN, 8, '0', 2));
#elif defined(KS0066)
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"R=");
	ks0066WriteString(mkNumString(rc5Buf, 14, '0', 2));
	ks0066SetXY(0, 1);
	ks0066WriteString((uint8_t*)"TB=");
	ks0066WriteString(mkNumString(((rc5Buf & 0x0800) > 0), 1, '0', 16));
	ks0066WriteString((uint8_t*)",RC=");
	ks0066WriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	ks0066WriteString((uint8_t*)",CM=");
	ks0066WriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));
#endif
}

void showRadio(uint8_t num)
{
	uint16_t freq = tunerFreqAvail();

#if defined(KS0108)
	uint8_t i;

	/* Frequency value */
	ks0108LoadFont(font_ks0066_ru_24, 1);
	ks0108SetXY(0, 0);
	ks0108WriteString((uint8_t*)"FM ");
	ks0108WriteString(mkNumString(freq / 100, 3, ' ', 10));
	ks0108WriteString((uint8_t*)"\x7F.\x7F");
	ks0108WriteString(mkNumString(freq / 10 % 10, 1, ' ', 10));
	ks0108LoadFont(font_ks0066_ru_08, 1);

	/* Signal level */
	ks0108SetXY (112, 0);
	for (i = 0; i < 16; i+=2) {
		if (i <= tunerLevel())
			ks0108WriteData(256 - (1<<(7 - i / 2)));
		else
			ks0108WriteData(0x80);
		ks0108WriteData(0x00);
	}

	/* Stereo indicator */
	ks0108SetXY(114, 2);
	if (tunerStereo())
		ks0108WriteString((uint8_t*)"ST");
	else
		ks0108WriteString((uint8_t*)"  ");

	/* Frequency scale */
	showBar(FM_FREQ_MIN>>4, FM_FREQ_MAX>>4, freq>>4);

	/* Station number */
	if (num) {
		showParValue(num);
	} else {
		ks0108LoadFont(font_ks0066_ru_24, 1);
		ks0108SetXY(93, 4);
		ks0108WriteString((uint8_t*)" --");
		ks0108LoadFont(font_ks0066_ru_08, 1);
	}
#elif defined(KS0066)
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
		ks0066WriteString((uint8_t*)"\x06");
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
#endif
}

void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels)
{
#if defined(KS0108)
	ks0108LoadFont(font_ks0066_ru_24, 1);
	ks0108SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0108SetXY(0, 4);
	if (value)
		writeStringEeprom(txtLabels[LABEL_ON]);
	else
		writeStringEeprom(txtLabels[LABEL_OFF]);
	ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(KS0066)
	ks0066SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0066SetXY(1, 1);
	if (value)
		writeStringEeprom(txtLabels[LABEL_ON]);
	else
		writeStringEeprom(txtLabels[LABEL_OFF]);
#endif
}

/* Show audio parameter */
void showSndParam(sndParam *param, uint8_t **txtLabels)
{
	showBar(param->min, param->max, param->value);
	showParValue(((int16_t)(param->value) * param->step + 4) >> 3);
	showParLabel(param->label, txtLabels);
}

#if defined(KS0108)
static void drawTm(timeMode tm, const uint8_t *font)
{
	if (getEtm() == tm)
		ks0108LoadFont(font, 0);
	else
		ks0108LoadFont(font, 1);
	ks0108WriteString(mkNumString(getTime(tm), 2, '0', 10));
	ks0108LoadFont(font, 1);
}
#endif

void showTime(uint8_t **txtLabels)
{
	readTime();
#if defined(KS0108)
	ks0108SetXY(4, 0);

	drawTm(HOUR, font_digits_32);
	ks0108WriteString((uint8_t*)"\x7F:\x7F");
	drawTm(MIN, font_digits_32);
	ks0108WriteString((uint8_t*)"\x7F:\x7F");
	drawTm(SEC, font_digits_32);

	ks0108SetXY(9, 4);

	drawTm(DAY, font_ks0066_ru_24);
	ks0108WriteString((uint8_t*)"\x7F.\x7F");
	drawTm(MONTH, font_ks0066_ru_24);
	ks0108WriteString((uint8_t*)"\x7F.\x7F");
	if (getEtm() == YEAR)
		ks0108LoadFont(font_ks0066_ru_24, 0);
	ks0108WriteString((uint8_t*)"20");
	ks0108WriteChar('\x7F');
	drawTm(YEAR, font_ks0066_ru_24);

	ks0108LoadFont(font_ks0066_ru_08, 1);
	ks0108SetXY(32, 7);

#elif defined(KS0066)
	ks0066SetXY(0, 0);

	ks0066WriteString(mkNumString(getTime(HOUR), 2, '0', 10));
	ks0066WriteData(':');
	ks0066WriteString(mkNumString(getTime(MIN), 2, '0', 10));
	ks0066WriteData(':');
	ks0066WriteString(mkNumString(getTime(SEC), 2, '0', 10));

	ks0066SetXY(11, 0);
	ks0066WriteString(mkNumString(getTime(DAY), 2, '0', 10));
	ks0066WriteData('.');
	ks0066WriteString(mkNumString(getTime(MONTH), 2, '0', 10));

	ks0066SetXY(12, 1);
	ks0066WriteString(mkNumString(2000 + getTime(YEAR), 4, '0', 10));

	ks0066SetXY(0, 1);
#endif

	switch (getTime(WEEK)) {
	case 1:
		writeStringEeprom(txtLabels[LABEL_THUESDAY]);
		break;
	case 2:
		writeStringEeprom(txtLabels[LABEL_WEDNESDAY]);
		break;
	case 3:
		writeStringEeprom(txtLabels[LABEL_THURSDAY]);
		break;
	case 4:
		writeStringEeprom(txtLabels[LABEL_FRIDAY]);
		break;
	case 5:
		writeStringEeprom(txtLabels[LABEL_SADURDAY]);
		break;
	case 6:
		writeStringEeprom(txtLabels[LABEL_SUNDAY]);
		break;
	case 7:
		writeStringEeprom(txtLabels[LABEL_MONDAY]);
		break;
	}

#if defined(KS0066)
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
#endif
	return;
}

void drawSpectrum(uint8_t *buf)
{
#if defined(KS0108)
	uint8_t i, j, k;
	int8_t row;
	uint8_t data;
	uint8_t val;
	ks0108SetXY(0, 0);
	for (i = 0; i < KS0108_ROWS; i++) {
		for (j = 0, k = 32; j < 32; j++, k++) {
			switch (spMode) {
			case SP_MODE_STEREO:
				if (i < KS0108_ROWS / 2) {
					val = buf[j];
					row = 3 - val / 8;
				} else {
					val = buf[k];
					row = 7 - val / 8;
				}
				break;
			default:
				val = buf[j] + buf[k];
				row = 7 - val / 8;
				break;
			}
			data = 0xFF;
			if (i == row)
				data = 0xFF << (7 - val % 8);
			else if (i < row)
				data = 0x00;
				ks0108WriteData(data);
				ks0108WriteData(data);
				ks0108WriteData(data);
				ks0108WriteData(0x00);
		}
	}
#elif defined(KS0066)
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
#endif
	return;
}

void loadDispParams(void)
{
	backlight = eeprom_read_byte(eepromBCKL);
	setBacklight(backlight);
	spMode  = eeprom_read_byte(eepromSpMode);
	defDisplay = eeprom_read_byte(eepromDisplay);

	return;
}

void saveDisplayParams(void)
{
	eeprom_write_byte(eepromBCKL, backlight);
	eeprom_write_byte(eepromSpMode, spMode);
	eeprom_write_byte(eepromDisplay, defDisplay);
}

/* Turn on/off backlight */
void setBacklight(int8_t backlight)
{
	if (backlight)
		DISPLAY_BACKLIGHT_PORT |= DISPLAY_BCKL;
	else
		DISPLAY_BACKLIGHT_PORT &= ~DISPLAY_BCKL;

	return;
}

/* Change backlight status */
void switchBacklight(void)
{
	backlight = !backlight;
	setBacklight(backlight);

	return;
}

/* Change spectrum mode */
void switchSpMode()
{
	spMode = !spMode;

	return;
}
