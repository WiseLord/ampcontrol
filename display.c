#include "display.h"

#include "audio.h"
#include "tea5767.h"
#include "eeprom.h"
#include "input.h"

uint8_t backlight;
uint8_t dig[17];		/* Array for num->string convert */

#ifdef KS0066
static uint8_t userSybmols = LCD_LEVELS;
#endif

void clearDisplay()
{
#ifdef KS0108
	gdFill(0x00);
#else
	lcdClear();
#endif
}

#ifndef KS0108
void lcdGenLevels(void)
{
	lcdWriteCommand(KS0066_SET_CGRAM);

	uint8_t i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (i + j >= 7) {
				lcdWriteData(0xFF);
			} else {
				lcdWriteData(0x00);
			}
		}
	}
}
void lcdGenBar(void)
{
	lcdWriteCommand(KS0066_SET_CGRAM);

	uint8_t i;
	uint8_t pos[] = {0x00, 0x10, 0x14, 0x15, 0x05, 0x01};

	for (i = 0; i < 48; i++) {
		if ((i & 0x07) == 0x03) {
			lcdWriteData(0x15);
		} else if ((i & 0x07) == 0x07) {
			lcdWriteData(0x00);
		} else {
			lcdWriteData(pos[i>>3]);
		}
	}
	/* Stereo indicator */
	lcdWriteData(0b00000000);
	lcdWriteData(0b00011011);
	lcdWriteData(0b00010101);
	lcdWriteData(0b00010101);
	lcdWriteData(0b00010101);
	lcdWriteData(0b00011011);
	lcdWriteData(0b00000000);
	lcdWriteData(0b00000000);
}
#endif

void displayInit()
{
#ifdef KS0108
	gdInit();
	gdLoadFont(font_ks0066_ru_08, 1);
#else
	lcdInit();
	lcdGenLevels();
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
		dig[i] = lead;
	dig[width] = '\0';
	i = width - 1;
	while (number > 0 || i == width - 1) {
		numdiv = number % radix;
		dig[i] = numdiv + 0x30;
		if (numdiv >= 10)
			dig[i] += 7;
		i--;
		number /= radix;
	}
	if (i >= 0)
		dig[i] = sign;
	return dig;
}

void showRC5Info(uint16_t rc5Buf)
{
#ifdef KS0108
	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(0, 0);
	gdWriteString((uint8_t*)"RC5:");
	gdSetXY(5, 1);
	gdWriteString((uint8_t*)"Raw = ");
	gdWriteString(mkNumString(rc5Buf, 14, '0', 2));
	gdSetXY(5, 2);
	gdWriteString((uint8_t*)"Tog = ");
	gdWriteString(mkNumString(((rc5Buf & 0x0800) > 0), 1, '0', 16));
	gdSetXY(5, 3);
	gdWriteString((uint8_t*)"Adr = ");
	gdWriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	gdSetXY(5, 4);
	gdWriteString((uint8_t*)"Cmd = ");
	gdWriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));
	gdSetXY(0, 6);
	gdWriteString((uint8_t*)"Buttons:");
	gdSetXY(5, 7);
	gdWriteString(mkNumString(BTN_PIN, 8, '0', 2));
#else
	lcdSetXY(0, 0);
	lcdWriteString((uint8_t*)"R=");
	lcdWriteString(mkNumString(rc5Buf, 14, '0', 2));
	lcdSetXY(0, 1);
	lcdWriteString((uint8_t*)"TB=");
	lcdWriteString(mkNumString(((rc5Buf & 0x0800) > 0), 1, '0', 16));
	lcdWriteString((uint8_t*)",RC=");
	lcdWriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	lcdWriteString((uint8_t*)",CM=");
	lcdWriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));
#endif
}

void showRadio(uint8_t *buf, uint8_t num)
{
	uint16_t freq = tea5767FreqAvail(buf);

#ifdef KS0108
	uint8_t i;

	/* Frequency value */
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteString((uint8_t*)"FM ");
	gdWriteString(mkNumString(freq / 100, 3, ' ', 10));
	gdWriteString((uint8_t*)"\x7F.\x7F");
	gdWriteString(mkNumString(freq / 10 % 10, 1, ' ', 10));
	gdLoadFont(font_ks0066_ru_08, 1);

	/* Signal level */
	gdSetXY (112, 0);
	for (i = 0; i < 16; i+=2) {
		if (i <= tea5767ADCLevel(buf))
			gdWriteData(256 - (1<<(7 - i / 2)));
		else
			gdWriteData(0x80);
		gdWriteData(0x00);
	}

	/* Stereo indicator */
	gdSetXY(114, 2);
	if (TEA5767_BUF_STEREO(buf))
		gdWriteString((uint8_t*)"ST");
	else
		gdWriteString((uint8_t*)"  ");

	/* Frequency scale */
	showBar(FM_FREQ_MIN>>4, FM_FREQ_MAX>>4, freq>>4);

	/* Station number */
	if (num) {
		showParValue(num);
	} else {
		gdLoadFont(font_ks0066_ru_24, 1);
		gdSetXY(93, 4);
		gdWriteString((uint8_t*)" --");
		gdLoadFont(font_ks0066_ru_08, 1);
	}
#else
	uint8_t lev;

	/* Frequency value */
	lcdSetXY(0, 0);
	lcdWriteString((uint8_t*)"FM ");
	lcdWriteString(mkNumString(freq / 100, 3, ' ', 10));
	lcdWriteData('.');
	lcdWriteString(mkNumString(freq / 10 % 10, 1, ' ', 10));

	/* Stereo indicator */
	lcdSetXY(9, 0);
	if (TEA5767_BUF_STEREO(buf))
		lcdWriteString((uint8_t*)"\x06");
	else
		lcdWriteString((uint8_t*)" ");

	/* Signal level */
	lcdSetXY(11, 0);
	lev = tea5767ADCLevel(buf) * 2 / 5;
	if (lev < 3) {
		lcdWriteData(lev);
		lcdWriteData(0x00);
	} else {
		lcdWriteData(0x03);
		lcdWriteData(lev - 3);
	}


	/* Frequency scale */
	showBar(FM_FREQ_MIN>>4, FM_FREQ_MAX>>4, freq>>4);

	/* Station number */
	lcdSetXY(14, 0);
	if (num) {
		lcdWriteString(mkNumString(num, 2, ' ', 10));
	} else {
		lcdWriteString((uint8_t*)"--");
	}
#endif
}

void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels)
{
#ifdef KS0108
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteStringEeprom(parLabel);
	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(116, 7);
	gdWriteStringEeprom(txtLabels[LABEL_DB]);
#else
	lcdSetXY(0, 0);
	lcdWriteStringEeprom(parLabel);
	lcdSetXY(14, 0);
	lcdWriteStringEeprom(txtLabels[LABEL_DB]);
#endif
}

void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels)
{
#ifdef KS0108
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteStringEeprom(parLabel);
	gdSetXY(0, 4);
	if (value)
		gdWriteStringEeprom(txtLabels[LABEL_ON]);
	else
		gdWriteStringEeprom(txtLabels[LABEL_OFF]);
	gdLoadFont(font_ks0066_ru_08, 1);
#else
	lcdSetXY(0, 0);
	lcdWriteStringEeprom(parLabel);
	lcdSetXY(1, 1);
	if (value)
		lcdWriteStringEeprom(txtLabels[LABEL_ON]);
	else
		lcdWriteStringEeprom(txtLabels[LABEL_OFF]);
#endif
}

void showBar(int16_t min, int16_t max, int16_t value)
{
#ifdef KS0108
	uint8_t data;
	uint8_t i, j;

	if (min + max) {
		value = (int16_t)85 * (value - min) / (max - min);
	} else {
		value = (int16_t)42 * value / max;
	}
	for (j = 5; j <= 6; j++) {
		gdSetXY(0, j);
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
			gdWriteData(data);
		}
	}
#else
	uint8_t i;

	if (userSybmols != LCD_BAR) {
		lcdGenBar();
		userSybmols = LCD_BAR;
	}

	lcdSetXY(0, 1);

	if (min + max) {
		value = (int16_t)48 * (value - min) / (max - min);
		for (i = 0; i < 16; i++) {
			if (value / 3 > i) {
				lcdWriteData(0x03);
			} else {
				if (value / 3 < i) {
					lcdWriteData(0x00);
				} else {
					lcdWriteData(value % 3);
				}
			}
		}
	} else {
		value = (int16_t)23 * value / max;
		if (value >= 0) {
			value++;
			for (i = 0; i < 7; i++) {
				lcdWriteData(0x00);
			}
			lcdWriteData(0x05);
			for (i = 0; i < 8; i++) {
				if (value / 3 > i) {
					lcdWriteData(0x03);
				} else {
					if (value / 3 < i) {
						lcdWriteData(0x00);
					} else {
						lcdWriteData(value % 3);
					}
				}
			}
		} else {
			value += 23;
			for (i = 0; i < 8; i++) {
				if (value / 3 > i) {
					lcdWriteData(0x00);
				} else {
					if (value / 3 < i) {
						lcdWriteData(0x03);
					} else {
						lcdWriteData(value % 3 + 3);
					}
				}
			}
			lcdWriteData(0x01);
			for (i = 0; i < 7; i++) {
				lcdWriteData(0x00);
			}
		}
	}
#endif
}

void showParValue(int8_t value)
{
#ifdef KS0108
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(93, 4);
	gdWriteString(mkNumString(value, 3, ' ', 10));
	gdLoadFont(font_ks0066_ru_08, 1);
#else
	lcdSetXY(11, 0);
	lcdWriteString(mkNumString(value, 3, ' ', 10));
#endif
}


void drawTm(timeMode tm, const uint8_t *font)
{
#ifdef KS0108
	if (etm == tm)
		gdLoadFont(font, 0);
	else
		gdLoadFont(font, 1);
	gdWriteString(mkNumString(time[tm], 2, '0', 10));
	gdLoadFont(font, 1);
#else
#endif
}

void showTime(uint8_t **txtLabels)
{
	getTime();
#ifdef KS0108
	gdSetXY(4, 0);

	drawTm(HOUR, font_digits_32);
	gdWriteString((uint8_t*)"\x7F:\x7F");
	drawTm(MIN, font_digits_32);
	gdWriteString((uint8_t*)"\x7F:\x7F");
	drawTm(SEC, font_digits_32);

	gdSetXY(9, 4);

	drawTm(DAY, font_ks0066_ru_24);
	gdWriteString((uint8_t*)"\x7F.\x7F");
	drawTm(MONTH, font_ks0066_ru_24);
	gdWriteString((uint8_t*)"\x7F.\x7F");
	if (etm == YEAR)
		gdLoadFont(font_ks0066_ru_24, 0);
	gdWriteString((uint8_t*)"20");
	gdWriteChar('\x7F');
	drawTm(YEAR, font_ks0066_ru_24);

	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(32, 7);

	switch (time[WEEK]) {
	case 1:
		gdWriteStringEeprom(txtLabels[LABEL_THUESDAY]);
		break;
	case 2:
		gdWriteStringEeprom(txtLabels[LABEL_WEDNESDAY]);
		break;
	case 3:
		gdWriteStringEeprom(txtLabels[LABEL_THURSDAY]);
		break;
	case 4:
		gdWriteStringEeprom(txtLabels[LABEL_FRIDAY]);
		break;
	case 5:
		gdWriteStringEeprom(txtLabels[LABEL_SADURDAY]);
		break;
	case 6:
		gdWriteStringEeprom(txtLabels[LABEL_SUNDAY]);
		break;
	case 7:
		gdWriteStringEeprom(txtLabels[LABEL_MONDAY]);
		break;
	default:
		break;
	}
#else
	lcdSetXY(0, 0);

	lcdWriteString(mkNumString(time[HOUR], 2, '0', 10));
	lcdWriteData(':');
	lcdWriteString(mkNumString(time[MIN], 2, '0', 10));
	lcdWriteData(':');
	lcdWriteString(mkNumString(time[SEC], 2, '0', 10));

	lcdSetXY(11, 0);
	lcdWriteString(mkNumString(time[DAY], 2, '0', 10));
	lcdWriteData('.');
	lcdWriteString(mkNumString(time[MONTH], 2, '0', 10));

	lcdSetXY(12, 1);
	lcdWriteString(mkNumString(2000 + time[YEAR], 4, '0', 10));

	lcdSetXY(0, 1);
	switch (time[WEEK]) {
	case 1:
		lcdWriteStringEeprom(txtLabels[LABEL_THUESDAY]);
		break;
	case 2:
		lcdWriteStringEeprom(txtLabels[LABEL_WEDNESDAY]);
		break;
	case 3:
		lcdWriteStringEeprom(txtLabels[LABEL_THURSDAY]);
		break;
	case 4:
		lcdWriteStringEeprom(txtLabels[LABEL_FRIDAY]);
		break;
	case 5:
		lcdWriteStringEeprom(txtLabels[LABEL_SADURDAY]);
		break;
	case 6:
		lcdWriteStringEeprom(txtLabels[LABEL_SUNDAY]);
		break;
	case 7:
		lcdWriteStringEeprom(txtLabels[LABEL_MONDAY]);
		break;
	default:
		break;
	}

	if (etm == NOEDIT) {
		lcdWriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	} else {
		switch (etm) {
		case HOUR:
			lcdSetXY(1, 0);
			break;
		case MIN:
			lcdSetXY(4, 0);
			break;
		case SEC:
			lcdSetXY(7, 0);
			break;
		case DAY:
			lcdSetXY(12, 0);
			break;
		case MONTH:
			lcdSetXY(15, 0);
			break;
		case YEAR:
			lcdSetXY(15, 1);
			break;
		default:
			break;
		}
		lcdWriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON | KS0066_CUR_BLINK_ON);
	}
#endif
	return;
}

void drawSpectrum(uint8_t *buf, uint8_t mode)
{
#ifdef KS0108
	uint8_t i, j, k;
	int8_t row;
	uint8_t data;
	uint8_t val;
	gdSetXY(0, 0);
	for (i = 0; i < GD_ROWS; i++) {
		for (j = 0, k = 32; j < 32; j++, k++) {
			switch (mode) {
			case SP_MODE_STEREO:
				if (i < GD_ROWS / 2) {
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
				gdWriteData(data);
				gdWriteData(data);
				gdWriteData(data);
				gdWriteData(0x00);
		}
	}
#else
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
	lcdSetXY(0, 0);
	for (i = 0; i < 16; i++) {
		data = ' ';
		if (lcdBuf[i] >= 8)
			data = lcdBuf[i] - 8;
		lcdWriteData(data);
	}
	lcdSetXY(0, 1);
	for (i = 0; i < 16; i++) {
		data = 0xFF;
		if (lcdBuf[i] < 8)
			data = lcdBuf[i];
		lcdWriteData(data);
	}
#endif
	return;
}

