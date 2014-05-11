#include "display.h"

#include "audio.h"
#include "tea5767.h"
#include "ks0108.h"
#include "eeprom.h"
#include "input.h"

void showRC5Info(uint16_t rc5Buf)
{
	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(0, 0);
	gdWriteString((uint8_t*)"RC5 command");
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
	gdWriteString((uint8_t*)"Buttons/Encoder");
	gdSetXY(5, 7);
	gdWriteString(mkNumString(BTN_PIN, 8, '0', 2));
}

void showRadio(uint8_t *buf, uint8_t num)
{
	uint16_t freq = tea5767FreqAvail(buf) / 10000;
	uint8_t i;

	/* Frequency value */
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteString((uint8_t*)"FM ");
	gdWriteString(mkNumString(freq/100, 3, ' ', 10));
	gdWriteChar('\x7F');
	gdWriteChar('.');
	gdWriteChar('\x7F');
	gdWriteString(mkNumString(freq/10%10, 1, ' ', 10));
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
	if (tea5767Stereo(buf))
		gdWriteString((uint8_t*)"ST");
	else
		gdWriteString((uint8_t*)"  ");

	/* Frequency scale */
	showBar(FM_FREQ_MIN>>4, FM_FREQ_MAX>>4, freq>>4);

	/* Station number */
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(100, 4);
	if (num)
		showParValue(num);
	else
		gdWriteString((uint8_t*)"--");
}

void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels)
{
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteStringEeprom(parLabel);
	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(116, 7);
	gdWriteStringEeprom(txtLabels[LABEL_DB]);
}

void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels)
{
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteStringEeprom(parLabel);
	gdSetXY(0, 4);
	if (value)
		gdWriteStringEeprom(txtLabels[LABEL_ON]);
	else
		gdWriteStringEeprom(txtLabels[LABEL_OFF]);
	gdLoadFont(font_ks0066_ru_08, 1);
}

void showBar(int16_t min, int16_t max, int16_t value)
{
	uint8_t i, j, data;

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
}

void showParValue(int8_t value)
{
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(93, 4);
	gdWriteString(mkNumString(value, 3, ' ', 10));
	gdLoadFont(font_ks0066_ru_08, 1);
}


void drawTm(timeMode tm, const uint8_t *font)
{
	if (etm == tm)
		gdLoadFont(font, 0);
	else
		gdLoadFont(font, 1);
	gdWriteString(mkNumString(time[tm], 2, '0', 10));
	gdLoadFont(font, 1);
}

void showTime(uint8_t **txtLabels)
{
	getTime();
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

	return;
}

void drawSpectrum(uint8_t *buf, uint8_t mode)
{
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
	return;
}

