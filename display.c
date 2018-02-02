#include "display.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "tuner/tuner.h"
#include "adc.h"
#include "pins.h"

static uint8_t _br;
static int8_t brStby;									/* Brightness in standby mode */
static int8_t brWork;									/* Brightness in working mode */

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

#if defined(KS0066) || defined(PCF8574)
static uint8_t userSybmols = LCD_LEVELS;
#endif

static void writeStringEeprom(const uint8_t *string)
{
	uint8_t i = 0;

	for (i = 0; i < STR_BUFSIZE; i++)
		strbuf[i] = eeprom_read_byte(&string[i]);

#if defined(KS0108)
	ks0108WriteString(strbuf);
#elif defined(KS0066) || defined(PCF8574)
	ks0066WriteString(strbuf);
#elif defined(LS020)
	ls020WriteString(strbuf);
#endif

	return;
}

#if defined(KS0066) || defined(PCF8574)
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
	/* Stereo indicator */
	ks0066WriteData(0b00000000);
	ks0066WriteData(0b00011011);
	ks0066WriteData(0b00010101);
	ks0066WriteData(0b00010101);
	ks0066WriteData(0b00010101);
	ks0066WriteData(0b00011011);
	ks0066WriteData(0b00000000);
	ks0066WriteData(0b00000000);

	return;
}
#endif

void displayInit()
{
#if defined(KS0108)
	ks0108Init();
	ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(KS0066) || defined(PCF8574)
	ks0066Init();
	lcdGenLevels();
#elif defined(LS020)
	ls020Init();
#endif
	DDR(BACKLIGHT) |= BACKLIGHT_LINE;

	return;
}

ISR (TIMER0_OVF_vect)
{
	ADCSRA |= 1<<ADSC;								/* Start ADC every second interrupt */

	static uint8_t br;

	if (++br >= DISP_MAX_BR)						/* Loop brightness */
		br = DISP_MIN_BR;

	if (br == _br) {
		PORT(BACKLIGHT) &= ~BACKLIGHT_LINE;		/* Turn backlight off */
	} else if (br == 0)
		PORT(BACKLIGHT) |= BACKLIGHT_LINE;		/* Turn backlight on */

	return;
}

void clearDisplay()
{
#if defined(KS0108)
	ks0108Clear();
#elif defined(KS0066) || defined(PCF8574)
	ks0066Clear();
#elif defined(LS020)
	ls020Clear();
#endif

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
#elif defined(KS0066) || defined(PCF8574)
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
#elif defined(LS020)
	uint8_t i;

	if (min + max) {
		value = (int16_t)85 * (value - min) / (max - min);
	} else {
		value = (int16_t)42 * value / max;
	}
	ls020SetXY(0, 60);
	for (i = 0; i < 85; i++) {
		if (((min + max) && (value <= i)) || (!(min + max) &&
			(((value > 0) && ((i < 42) || (value + 42 < i))) ||
			((value <= 0) && ((i > 42) || (value + 42 > i))))))
		{
			ls020DrawRect(i * 2 + 2, 55, i * 2 + 2, 85, COLOR_BCKG);
			ls020DrawRect(i * 2 + 2, 69, i * 2 + 2, 71, COLOR_BLUE);
		} else {
			ls020DrawRect(i * 2 + 2, 55, i * 2 + 2, 85, COLOR_CYAN);
		}
	}
#endif

	return;
}

static void showParValue(int8_t value)
{
#if defined(KS0108)
	ks0108LoadFont(font_ks0066_ru_24, 1);
	ks0108SetXY(93, 4);
	ks0108WriteString(mkNumString(value, 3, ' ', 10));
	ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined(KS0066) || defined(PCF8574)
	ks0066SetXY(11, 0);
	ks0066WriteString(mkNumString(value, 3, ' ', 10));
#elif defined(LS020)
	ls020LoadFont(font_digits_32, COLOR_CYAN, 1);
	ls020SetXY(100, 96);
	ls020WriteString(mkNumString(value, 3, ' ', 10));
#endif

	return;
}

static void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels)
{
#if defined(KS0108)
	ks0108LoadFont(font_ks0066_ru_24, 1);
	ks0108SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0108LoadFont(font_ks0066_ru_08, 1);
#elif defined (KS0066) || defined(PCF8574)
	ks0066SetXY(0, 0);
	writeStringEeprom(parLabel);
#elif defined(LS020)
	ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
	ls020SetXY(4, 8);
	writeStringEeprom(parLabel);
#endif

	return;
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
	ks0108WriteString(mkNumString(INPUT_PIN, 8, '0', 2));
#elif defined(KS0066) || defined(PCF8574)
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
#elif defined(LS020)
	ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
	ls020SetXY(4, 0);
	ls020WriteString(mkNumString(rc5Buf, 14, '0', 2));
	ls020SetXY(5, 24);
	ls020WriteString((uint8_t*)"Tog = ");
	ls020WriteString(mkNumString(((rc5Buf & 0x0800) > 0), 1, '0', 16));
	ls020SetXY(6, 48);
	ls020WriteString((uint8_t*)"Adr = ");
	ls020WriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	ls020SetXY(4, 72);
	ls020WriteString((uint8_t*)"Cmd = ");
	ls020WriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));
	ls020SetXY(6, 104);
	ls020WriteString((uint8_t*)"Btn = ");
	ls020WriteString(mkNumString(INPUT_PIN, 8, '0', 2));
#endif

	return;
}

#if !defined(NOTUNER)
void showRadio(void)
{
	uint16_t freq = tunerGetFreq();
	uint8_t num = tunerStationNum();

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
			ks0108WriteData(256 - (128>>(i / 2)));
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
#elif defined(KS0066) || defined(PCF8574)
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
#elif defined(LS020)

	/* Frequency value */
	ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
	ls020SetXY(4, 8);
	ls020WriteString((uint8_t*)"FM ");
	ls020WriteString(mkNumString(freq / 100, 3, ' ', 10));
	ls020WriteString((uint8_t*)"\x7F.\x7F");
	ls020WriteString(mkNumString(freq / 10 % 10, 1, ' ', 10));

	/* Stereo indicator */
	ls020LoadFont(font_ks0066_ru_08, COLOR_CYAN, 1);
	ls020SetXY(132, 12);
	if (tunerStereo())
		ls020WriteString((uint8_t*)"STEREO");
	else
		ls020WriteString((uint8_t*)"      ");

	/* Frequency scale */
	showBar(FM_FREQ_MIN>>4, FM_FREQ_MAX>>4, freq>>4);

	/* Station number */
	ls020LoadFont(font_digits_32, COLOR_CYAN, 1);
	ls020SetXY(124, 96);
	if (num) {
		ls020WriteString(mkNumString(num, 3, ' ', 10));
	} else {
		ls020WriteString((uint8_t*)" --");
	}
#endif

	return;
}
#endif

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
#elif defined(KS0066) || defined(PCF8574)
	ks0066SetXY(0, 0);
	writeStringEeprom(parLabel);
	ks0066SetXY(1, 1);
	if (value)
		writeStringEeprom(txtLabels[LABEL_ON]);
	else
		writeStringEeprom(txtLabels[LABEL_OFF]);
#elif defined(LS020)
	ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
	ls020SetXY(4, 0);
	writeStringEeprom(parLabel);
	ls020SetXY(4, 32);
	if (value)
		writeStringEeprom(txtLabels[LABEL_ON]);
	else
		writeStringEeprom(txtLabels[LABEL_OFF]);
#endif

	return;
}

/* Show brightness control */
void showBrWork(uint8_t **txtLabels, uint8_t *buf)
{
	showBar(DISP_MIN_BR, DISP_MAX_BR, brWork);
	showParValue(brWork);
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
	showParValue(((int16_t)(param->value) * param->step + 4) >> 3);
	showParLabel(param->label, txtLabels);

#if defined(KS0108)
	ks0108LoadFont(font_ks0066_ru_08, 1);
	ks0108SetXY(116, 7);
	writeStringEeprom(txtLabels[LABEL_DB]);
#elif defined (KS0066) || defined(PCF8574)
	ks0066SetXY(14, 0);
	writeStringEeprom(txtLabels[LABEL_DB]);
#elif defined(LS020)
	ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
	ls020SetXY(150, 104);
	writeStringEeprom(txtLabels[LABEL_DB]);
#endif

	return;
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

	return;
}
#elif defined(KS0066) || defined(PCF8574)
static void drawTm(timeMode tm)
{
	ks0066WriteString(mkNumString(getTime(tm), 2, '0', 10));

	return;
}
#elif defined(LS020)
static void drawTm(timeMode tm, const uint8_t *font, uint8_t mult)
{
	if (getEtm() == tm)
		ls020LoadFont(font, COLOR_RED, mult);
	else
		ls020LoadFont(font, COLOR_CYAN, mult);
	ls020WriteString(mkNumString(getTime(tm), 2, '0', 10));
	ls020LoadFont(font, COLOR_CYAN, mult);

	return;
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

#elif defined(KS0066) || defined(PCF8574)
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
#elif defined(LS020)
	ls020SetXY(2, 4);

	drawTm(HOUR, font_ks0066_ru_24, 2);
	ls020WriteString((uint8_t*)"\x7F:\x7F");
	drawTm(MIN, font_ks0066_ru_24, 2);
	ls020WriteString((uint8_t*)"\x7F:\x7F");
	drawTm(SEC, font_ks0066_ru_24, 2);

	ls020SetXY(12, 64);

	drawTm(DAY, font_digits_32, 1);
	ls020WriteString((uint8_t*)"\x7F.\x7F");
	drawTm(MONTH, font_digits_32, 1);
	ls020WriteString((uint8_t*)"\x7F.\x7F");
	if (getEtm() == YEAR)
		ls020LoadFont(font_digits_32, COLOR_RED, 1);
	ls020WriteString((uint8_t*)"20");
	ls020WriteChar('\x7F');
	drawTm(YEAR, font_digits_32, 1);

	ls020LoadFont(font_ks0066_ru_24, COLOR_CYAN, 1);
	ls020SetXY(24, 104);
#endif

	writeStringEeprom(txtLabels[LABEL_SUNDAY + getTime(WEEK) % 7]);

#if defined(KS0066) || defined(PCF8574)
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
			if (i == row) {
				data = 0xFF << (7 - val % 8);
			} else if (i < row) {
				data = 0x00;
			}
			ks0108WriteData(data);
			ks0108WriteData(data);
			ks0108WriteData(data);
			ks0108WriteData(0x00);
		}
	}
#elif defined(KS0066) || defined(PCF8574)
	uint8_t i;
	uint8_t lcdBuf[16];

	if (userSybmols != LCD_LEVELS) {
		lcdGenLevels();
		userSybmols = LCD_LEVELS;
	}

	for (i = 0; i < 16; i++) {
		lcdBuf[i] = buf[2 * i] + buf[2 * i + 1];
		lcdBuf[i] += buf[32 + 2 * i] + buf[32 + 2 * i + 1];
		lcdBuf[i] >>= 2;
	}

	for (i = 0; i < 16; i++) {
		ks0066SetXY(i, 0);
		if (lcdBuf[i] < 8)
			ks0066WriteData(0x20);
		else
			ks0066WriteData(lcdBuf[i] - 8);
		ks0066SetXY(i, 1);
		if (lcdBuf[i] < 8)
			ks0066WriteData(lcdBuf[i]);
		else
			ks0066WriteData(0xFF);
	}

#elif defined(LS020)
	uint8_t i;
	uint8_t val;

	for (i = 0; i < 29; i++) {
		switch (spMode) {
		case SP_MODE_STEREO:
			val = buf[i] * 2;
			ls020DrawRect(2 + i * 6, 2, 5 + i * 6, 2 + (63 - val) - 1, COLOR_BCKG);
			ls020DrawRect(2 + i * 6, 2 + (63 - val), 5 + i * 6, 65, COLOR_YELLOW);
			val = buf[i + 32] * 2;
			ls020DrawRect(2 + i * 6, 2 + 64, 5 + i * 6, 2 + (63 - val) - 1 + 64, COLOR_BCKG);
			ls020DrawRect(2 + i * 6, 2 + (63 - val) + 64, 5 + i * 6, 65 + 64, COLOR_YELLOW);
			break;
		default:
			val = (buf[i] + buf[i + 32]) * 2;
			ls020DrawRect(2 + i * 6, 2, 5 + i * 6, 2 + (127 - val) - 1, COLOR_BCKG);
			ls020DrawRect(2 + i * 6, 2 + (127 - val), 5 + i * 6, 129, COLOR_YELLOW);
			break;
		}
	}
#endif

	return;
}

void setWorkBrightness(void)
{
	_br = brWork;

#if defined(PCF8574)
	if (brWork == DISP_MAX_BR)
		pcf8574IntBacklight(BACKLIGHT_ON);
	else
		pcf8574IntBacklight(BACKLIGHT_OFF);
#endif

	return;
}

void setStbyBrightness(void)
{
	_br = brStby;

#if defined(PCF8574)
	pcf8574IntBacklight(BACKLIGHT_OFF);
#endif

	return;
}

void loadDispParams(void)
{
	brStby = eeprom_read_byte(eepromBrStby);
	brWork = eeprom_read_byte(eepromBrWork);
	spMode  = eeprom_read_byte(eepromSpMode);
	defDisplay = eeprom_read_byte(eepromDisplay);

	return;
}

void saveDisplayParams(void)
{
	eeprom_update_byte(eepromBrStby, brStby);
	eeprom_update_byte(eepromBrWork, brWork);
	eeprom_update_byte(eepromSpMode, spMode);
	eeprom_update_byte(eepromDisplay, defDisplay);

	return;
}

/* Change spectrum mode */
void switchSpMode()
{
	spMode = !spMode;

	return;
}
