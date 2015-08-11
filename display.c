#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "tuner/tuner.h"
#include "ds18x20.h"
#include "temp.h"
#include "adc.h"
#include "tuner/rds.h"

int8_t brStby;								/* Brightness in standby mode */
int8_t brWork;								/* Brightness in working mode */

uint8_t spMode;								/* Spectrum mode */
static uint8_t fallSpeed;					/* Spectrum fall speed */

static uint8_t rc5CmdInd = CMD_RC5_STBY;
static uint8_t rc5Cmd;
static uint8_t rc5Addr;

static uint8_t defDisplay;					/* Default display mode */

uint8_t *txtLabels[LABEL_END];				/* Array with text label pointers */

uint8_t strbuf[STR_BUFSIZE + 1];			/* String buffer */

#ifdef KS0066
static uint8_t userSybmols = LCD_LEVELS;	/* Generated user symbols for ks0066 */
static uint8_t userAddSym = SYM_STEREO;		/* Additional user symbol */
#endif

#ifdef KS0066
static void lcdGenLevels(void)
{
	uint8_t i, j;

	userSybmols = LCD_LEVELS;
	ks0066WriteCommand(KS0066_SET_CGRAM);

	for (i = 0; i < 7; i++)
		for (j = 0; j < 8; j++)
			if (i + j >= 7)
				ks0066WriteData(0xFF);
			else
				ks0066WriteData(0x00);

	return;
}

static void lcdGenBar(uint8_t sym)
{
	ks0066WriteCommand(KS0066_SET_CGRAM);

	uint8_t i;
	uint8_t pos[] = {0x00, 0x10, 0x14, 0x15, 0x05, 0x01};

	userSybmols = LCD_BAR;

	for (i = 0; i < 48; i++) {
		if ((i & 0x07) == 0x03) {
			ks0066WriteData(0x15);
		} else if ((i & 0x07) == 0x07) {
			ks0066WriteData(0x00);
		} else {
			ks0066WriteData(pos[i>>3]);
		}
	}
	switch (sym) {
	case SYM_MUTE:
		/* Speaker */
		ks0066WriteData(0b00001);
		ks0066WriteData(0b00011);
		ks0066WriteData(0b11101);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b11101);
		ks0066WriteData(0b00011);
		ks0066WriteData(0b00001);
		ks0066WriteData(0b00000);
		break;
	case SYM_LOUD:
		ks0066WriteData(0b00000);
		ks0066WriteData(0b10000);
		ks0066WriteData(0b10001);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b00000);
		break;
	default:
		/* Stereo indicator */
		ks0066WriteData(0b00000);
		ks0066WriteData(0b11011);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b10101);
		ks0066WriteData(0b11011);
		ks0066WriteData(0b00000);
		ks0066WriteData(0b00000);
		break;
	}
	/* Cross */
	ks0066WriteData(0b00000);
	ks0066WriteData(0b10001);
	ks0066WriteData(0b01010);
	ks0066WriteData(0b00100);
	ks0066WriteData(0b01010);
	ks0066WriteData(0b10001);
	ks0066WriteData(0b00000);
	ks0066WriteData(0b00000);

	return;
}
#endif

static void showBar(int16_t min, int16_t max, int16_t value)
{
#ifdef KS0066
	uint8_t i;

	if (userSybmols != LCD_BAR)
		lcdGenBar(userAddSym);

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
#else
	uint8_t i, j;
	uint8_t color;

	if (min + max) {
		value = (int16_t)91 * (value - min) / (max - min);
	} else {
		value = (int16_t)45 * value / max;
	}

	for (i = 0; i < 91; i++) {
		if (((min + max) && (value <= i)) || (!(min + max) &&
			(((value > 0) && ((i < 45) || (value + 45 < i))) ||
			((value <= 0) && ((i > 45) || (value + 45 > i)))))) {
			color = 0x00;
		} else {
			color = 0x01;
		}
		if (!(i & 0x01)) {
			for (j = 28; j < 39; j++) {
				if (j == 33) {
					gdDrawPixel(i, j, 1);
				} else {
					gdDrawPixel(i, j, color);
				}
			}
		}
	}
	return;
#endif
}

static void showParValue(int8_t value)
{
#ifdef KS0066
	ks0066SetXY(11, 0);
	writeNum(value, 3, ' ', 10);
#else
	gdLoadFont(font_ks0066_ru_24, 1, FONT_DIR_0);
	gdSetXY(94, 30);
	writeNum(value, 3, ' ', 10);
	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);
	return;

#endif
}
static void showParLabel(const uint8_t *parLabel)
{
#ifdef KS0066
	ks0066SetXY (0, 0);
	writeStringEeprom(parLabel);
#else
	gdLoadFont(font_ks0066_ru_24, 1, FONT_DIR_0);
	gdSetXY(0, 0);
	writeStringEeprom(parLabel);
	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);
	return;

#endif
}
#ifdef KS0066
#else
static void showParIcon(const uint8_t *icon)
{
	gdSetXY(104, 2);
	gdWriteIcon24(icon);

	return;
}
#endif

static void drawMiniSpectrum(void)
{
	volatile uint8_t *buf = getSpData(fallSpeed);

#ifdef KS0066
	uint16_t data;
	uint8_t i;

	if (userSybmols != LCD_BAR)
		lcdGenBar (userAddSym);
	data = 0;
	for (i = 0; i < FFT_SIZE / 2; i++) {
		data += buf[i];
		data += buf[FFT_SIZE / 2 + i];
	}
	data >>= 5;

	ks0066SetXY(0, 1);
	for (i = 0; i < KS0066_SCREEN_WIDTH; i++) {
		if (data / 3 > i) {
			ks0066WriteData(0x03);
		} else {
			if (data / 3 < i) {
				ks0066WriteData(0x00);
			} else {
				ks0066WriteData(data % 3);
			}
		}
	}
#else
	uint8_t x, xbase;
	uint8_t y, ybase;

	if (buf) {
		for (y = 0; y < GD_SIZE_Y / 8 * 4; y++) {
			for (x = 0; x < GD_SIZE_X / 4 - 1; x++) {
				xbase = x * 3;
				ybase = 63 - y;
				if (buf[x] + buf[x + 32] >= y * 2) {
					gdDrawPixel(xbase + 0, ybase, 1);
					gdDrawPixel(xbase + 1, ybase, 1);
				} else {
					gdDrawPixel(xbase + 0, ybase, 0);
					gdDrawPixel(xbase + 1, ybase, 0);
				}
			}
		}
	}

	return;
#endif
}

#ifdef KS0066
#else
static void drawBarSpectrum(void)
{
	uint8_t x, xbase;
	uint8_t y, ybase;

	volatile uint8_t *buf = getSpData(fallSpeed);

	if (buf) {
		for (y = 0; y < GD_SIZE_Y / 8 * 3; y++) {
			for (x = 0; x < GD_SIZE_X / 4 - 1; x++) {
				xbase = x * 3;
				ybase = 63 - y;
				if (buf[x] + buf[x + 32] >= y * 3) {
					gdDrawPixel(xbase + 0, ybase, 1);
					gdDrawPixel(xbase + 1, ybase, 1);
				} else {
					gdDrawPixel(xbase + 0, ybase, 0);
					gdDrawPixel(xbase + 1, ybase, 0);
				}
			}
		}
	}
	return;
}
#endif

#ifdef KS0066
static void drawTm(uint8_t tm)
#else
static void drawTm(uint8_t tm, const uint8_t *font)
#endif
{
	if (getEtm() == tm) {
#ifdef KS0066
#else
		gdLoadFont(font, 0, FONT_DIR_0);
#endif
	} else {
#ifdef KS0066
#else
		gdLoadFont(font, 1, FONT_DIR_0);
#endif
	}
	writeNum(getTime(tm), 2, '0', 10);
#ifdef KS0066
#else
	gdLoadFont(font, 1, FONT_DIR_0);
#endif

	return;
}

#ifdef KS0066
#else
static void drawAm(uint8_t am, const uint8_t *font)
{
	if (getEam() == am)
		gdLoadFont(font, 0, FONT_DIR_0);
	else
		gdLoadFont(font, 1, FONT_DIR_0);
	writeNum(getAlarm(am), 2, '0', 10);
	gdLoadFont(font, 1, FONT_DIR_0);

	return;
}
#endif

void displayInit(void)
{
	uint8_t i;
	uint8_t *addr;

	addr = labelsAddr;
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

#ifdef KS0066
	ks0066Init();
	lcdGenLevels();
#else
	gdInit();
#endif

	brStby = eeprom_read_byte(eepromBrStby);
	brWork = eeprom_read_byte(eepromBrWork);
	spMode  = eeprom_read_byte(eepromSpMode);
	defDisplay = eeprom_read_byte(eepromDisplay);
	fallSpeed = eeprom_read_byte(eepromFallSpeed);
	if (fallSpeed > FALL_SPEED_FAST)
		fallSpeed = FALL_SPEED_FAST;

	return;
}

void displayClear(void)
{
#ifdef KS0066
	ks0066Clear();
#else
	gdClear();
#endif

	return;
}

void writeString(uint8_t *string)
{
#ifdef KS0066
	ks0066WriteString (string);
#else
	gdWriteString(string);
#endif
	return;
}

void writeStringEeprom(const uint8_t *string)
{
	uint8_t i = 0;

	for (i = 0; i < STR_BUFSIZE; i++)
		strbuf[i] = eeprom_read_byte(&string[i]);

	writeString(strbuf);

	return;
}

void writeNum(int16_t number, uint8_t width, uint8_t lead, uint8_t radix)
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

	writeString(strbuf);

	return;
}

uint8_t **getTxtLabels(void)
{
	return txtLabels;
}

void setDefDisplay(uint8_t value)
{
	defDisplay = value;

	return;
}

uint8_t getDefDisplay()
{
	return defDisplay;
}

void nextRC5Cmd(void)
{
	eeprom_update_byte(eepromRC5Cmd + rc5CmdInd, rc5Cmd);
	eeprom_update_byte(eepromRC5Addr, rc5Addr);

	rc5CmdInd++;
	if (rc5CmdInd >= CMD_BTN_1)
		rc5CmdInd = CMD_RC5_STBY;

	rc5Addr = eeprom_read_byte(eepromRC5Addr);
	rc5Cmd = eeprom_read_byte(eepromRC5Cmd + rc5CmdInd);

	setRC5Buf(rc5Addr, rc5Cmd);

	return;
}

void startTestMode(void)
{
	rc5CmdInd = CMD_RC5_STBY;
	rc5Addr = eeprom_read_byte(eepromRC5Addr);
	rc5Cmd = eeprom_read_byte(eepromRC5Cmd + rc5CmdInd);

	setRC5Buf(rc5Addr, rc5Cmd);

	return;
}

void showRC5Info(void)
{
#ifdef KS0066
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"showRC5Info");
#else
	uint16_t rc5Buf = getRC5Buf();
	uint8_t btnBuf = getBtnBuf();
	uint8_t encBuf = getEncBuf();

	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);
	gdSetXY(0, 0);
	writeStringEeprom(txtLabels[LABEL_IN_STATUS]);
	gdSetXY(4, 9);
	writeStringEeprom(txtLabels[LABEL_REMOTE]);
	gdSetXY(45, 9);
	writeNum(rc5Buf, 14, '0', 2);

	gdSetXY(4, 18);
	writeStringEeprom(txtLabels[LABEL_BUTTONS]);
	gdSetXY(75, 18);
	writeNum(btnBuf, 5, '0', 2);
	gdSetXY(108, 18);
	writeString((uint8_t*)"/");
	gdSetXY(117, 18);
	writeNum(encBuf, 2, '0', 2);

	gdSetXY(0, 30);
	writeStringEeprom(txtLabels[LABEL_LEARN_MODE]);
	gdSetXY(4, 39);
	writeStringEeprom(txtLabels[LABEL_BUTTON]);
	gdSetXY(48, 39);
	writeStringEeprom(txtLabels[LABEL_RC5_STBY + rc5CmdInd]);

	gdSetXY(8, 48);
	writeStringEeprom(txtLabels[LABEL_ADDRESS]);
	gdSetXY(64, 48);
	rc5Addr = (rc5Buf & 0x07C0)>>6;
	writeNum(rc5Addr, 2, '0', 16);
	writeString((uint8_t*)" => ");
	writeNum(eeprom_read_byte(eepromRC5Addr), 2, '0', 16);

	gdSetXY(8, 56);
	writeStringEeprom(txtLabels[LABEL_COMMAND]);
	gdSetXY(64, 56);
	rc5Cmd = rc5Buf & 0x003F;
	writeNum(rc5Cmd, 2, '0', 16);
	writeString((uint8_t*)" => ");
	writeNum(eeprom_read_byte(eepromRC5Cmd + rc5CmdInd), 2, '0', 16);
#endif
	return;
}

void showTemp(void)
{
#ifdef KS0066
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"showTemp");
#else
	int8_t tempTH;

	tempTH = getTempTH();

	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);

	gdSetXY(0, 48);
	writeStringEeprom(txtLabels[LABEL_SENSOR]);
	writeString((uint8_t*)" 1: ");
	writeNum(ds18x20GetTemp(0), 3, ' ', 10);
	writeString((uint8_t*)" \xDF""C");

	gdSetXY(0, 56);
	writeStringEeprom(txtLabels[LABEL_SENSOR]);
	writeString((uint8_t*)" 2: ");
	writeNum(ds18x20GetTemp(1), 3, ' ', 10);
	writeString((uint8_t*)" \xDF""C");

	showParValue(tempTH);
	showBar(MIN_TEMP, MAX_TEMP, tempTH);
	showParLabel(txtLabels[LABEL_THRESHOLD]);
	showParIcon(icons_24_threshold);

	gdSetXY(118, 56);
	writeString((uint8_t*)"\xDF""C");
#endif
	return;
}

void showRadio(uint8_t tune)
{
	tunerReadStatus();

	uint8_t i;

	uint16_t freq = tunerGetFreq();
	uint8_t level = tunerLevel();
	uint8_t num = tunerStationNum();
	uint8_t favNum = tunerFavStationNum();
	static uint8_t rdsMode;

#ifdef KS0066
	if (userSybmols != LCD_BAR || userAddSym != SYM_STEREO)
		lcdGenBar(SYM_STEREO);

	/* Frequency value */
	ks0066SetXY(0, 0);
	writeString((uint8_t*)"FM ");
	writeNum(freq / 100, 3, ' ', 10);
	ks0066WriteData('.');
	writeNum(freq % 100, 2, '0', 10);

	/* Signal level */
	ks0066SetXY(12, 0);
	level = level * 2 / 5;
	if (level < 3) {
		ks0066WriteData(level);
		ks0066WriteData(0x00);
	} else {
		ks0066WriteData(0x03);
		ks0066WriteData(level - 3);
	}

	/* Stereo indicator */
	ks0066SetXY(10, 0);
	if (tunerStereo())
		writeString((uint8_t*)"\x06");
	else
		writeString((uint8_t*)" ");

	/* Favourite station number */
	ks0066SetXY(15, 0);
	if (favNum) {
		writeNum(favNum % 10, 1, ' ', 10);
	} else {
		writeString((uint8_t*)"-");
	}

	/* Station number */
	ks0066SetXY(14, 1);
	if (num) {
		writeNum(num, 2, ' ', 10);
	} else {
		writeString((uint8_t*)"--");
	}

	/* Select between RDS and spectrum mode */
	if (rdsMode) {
		/* RDS data */
		ks0066SetXY(0, 1);
		writeString((uint8_t*)"RDS:");
		writeString(rdsGetText ());
		rdsMode = 0;
	} else {
		/* Frequency scale */
		uint8_t value = (int16_t)36 * ((freq - FM_FREQ_MIN)>>4) / ((FM_FREQ_MAX - FM_FREQ_MIN)>>4);
		ks0066SetXY(0, 1);
		for (i = 0; i < 12; i++) {
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

	}
	/* Tune status */
	ks0066SetXY (12, 1);
	if (tune == MODE_RADIO_TUNE) {
		writeString((uint8_t*)"<>");
	} else {
		writeString((uint8_t*)"  ");
	}

#else
	/* Frequency value */
	gdLoadFont(font_ks0066_ru_24, 1, FONT_DIR_0);
	gdSetXY(0, 0);
	writeString((uint8_t*)"FM ");
	writeNum(freq / 100, 3, ' ', 10);
	writeString((uint8_t*)"\x7F.\x7F");
	writeNum(freq % 100, 2, '0', 10);
	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);

	/* Signal level */
	gdSetXY (112, 0);
	for (i = 0; i < 16; i+=2) {
		if (i <= level) {
			gdDrawLine(112 + i, 7, 112 + i, 7 - i / 2, 0x01);
		} else {
			gdDrawLine(112 + i, 6, 112 + i, 0, 0x00);
			gdDrawPixel(112 + i, 7, 0x01);
		}
	}

	/* Stereo indicator */
	gdSetXY(116, 12);
	if (tunerStereo())
		writeString((uint8_t*)"ST");
	else
		writeString((uint8_t*)"  ");

	/* Favourite station number */
	gdSetXY(114, 23);
	gdWriteChar(0xF5);						/* Heart symbol */
	gdSetXY(122, 23);
	if (favNum)
		writeNum(favNum % 10, 1, ' ', 10);
	else
		writeString((uint8_t*)"-");

	/* Station number */
	gdLoadFont(font_ks0066_ru_24, 1, FONT_DIR_0);
	gdSetXY(94, 30);
	if (num)
		writeNum(num, 3, ' ', 10);
	else
		writeString((uint8_t*)" --");
	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);

	/* Frequency scale */
	showBar(FM_FREQ_MIN>>4, FM_FREQ_MAX>>4, freq>>4);

	/* Select between RDS and spectrum mode */
	if (rdsMode) {
		gdLoadFont(font_ks0066_ru_24, 1, FONT_DIR_0);
		gdSetXY(0, 40);
		writeString(rdsGetText());
		gdDrawFilledRect(gdGetX(), 40, 103 - gdGetX(), 24, 0);
		rdsMode = 0;
	} else {
		drawBarSpectrum();
	}
	rdsMode = rdsGetFlag();

	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);
	if (tune == MODE_RADIO_TUNE) {
		gdSetXY(103, 56);
		writeString((uint8_t*)"\xDB\xDB\xD0\xDC\xDC");
	} else {
		gdSetXY(110, 56);
		if (rdsMode) {
			writeString((uint8_t*)"RDS");
		} else {
			writeString((uint8_t*)"   ");
		}
	}
#endif
	return;
}

void showMute(void)
{
	showParLabel(txtLabels[LABEL_MUTE]);
	drawMiniSpectrum();

#ifdef KS0066
	if (userAddSym != SYM_MUTE)
		lcdGenBar(SYM_MUTE);
	ks0066SetXY(14, 0);
	ks0066WriteData(0x06);
	if (sndGetMute ())
		ks0066WriteData(0x07);
	else
		ks0066WriteData(' ');
#else
	gdSetXY(96, 32);
	if (sndGetMute())
		gdWriteIcon32(icons_32_mute_on);
	else
		gdWriteIcon32(icons_32_mute_off);
#endif

	return;
}

void showLoudness(void)
{
	showParLabel(txtLabels[LABEL_LOUDNESS]);
	drawMiniSpectrum();
#ifdef KS0066
	if (userAddSym != SYM_LOUD)
		lcdGenBar(SYM_LOUD);
	ks0066SetXY(14, 0);
	ks0066WriteData(0x06);
	if (sndGetLoudness ())
		ks0066WriteData(0x07);
	else
		ks0066WriteData(' ');
#else
	gdSetXY(96, 32);
	if (sndGetLoudness())
		gdWriteIcon32(icons_32_loud_on);
	else
		gdWriteIcon32(icons_32_loud_off);
#endif
	return;
}

void showBrWork(void)
{
	showParLabel(txtLabels[LABEL_BR_WORK]);
	showBar(MIN_BRIGHTNESS, MAX_BRIGHTNESS, brWork);
#ifdef KS0066
	ks0066SetXY(13, 0);
	writeNum(brWork, 3, ' ', 10);
#else
	showParValue(brWork);
	drawBarSpectrum();
	showParIcon(icons_24_brightness);
#endif
	return;
}

void changeBrWork(int8_t diff)
{
	brWork += diff;
	if (brWork > MAX_BRIGHTNESS)
		brWork = MAX_BRIGHTNESS;
	if (brWork < MIN_BRIGHTNESS)
		brWork = MIN_BRIGHTNESS;
	setWorkBrightness();

	return;
}

void showSndParam(sndMode mode)
{
	sndParam *param = sndParAddr(mode);

	showParLabel(param->label);
	showParValue(((int16_t)(param->value) * (int8_t)pgm_read_byte(&param->grid->step) + 4) >> 3);
	showBar((int8_t)pgm_read_byte(&param->grid->min), (int8_t)pgm_read_byte(&param->grid->max), param->value);
#ifdef KS0066
	ks0066SetXY(14, 0);
#else
	drawBarSpectrum();
	showParIcon(param->icon);
	gdSetXY(116, 56);
#endif
	writeStringEeprom(txtLabels[LABEL_DB]);

	return;
}

void showTime(void)
{
#ifdef KS0066
	ks0066SetXY(0, 0);
	drawTm (DS1307_HOUR);
	ks0066WriteData (':');
	drawTm (DS1307_MIN);
	ks0066WriteData (':');
	drawTm (DS1307_SEC);

	ks0066SetXY(11, 0);
	drawTm(DS1307_DATE);
	ks0066WriteData('.');
	drawTm(DS1307_MONTH);

	ks0066SetXY(12, 1);
	ks0066WriteString((uint8_t*)"20");
	drawTm(DS1307_YEAR);

	ks0066SetXY(0, 1);
#else
	gdSetXY(4, 0);

	drawTm(DS1307_HOUR, font_digits_32);
	writeString((uint8_t*)"\x7F:\x7F");
	drawTm(DS1307_MIN, font_digits_32);
	writeString((uint8_t*)"\x7F:\x7F");
	drawTm(DS1307_SEC, font_digits_32);

	gdSetXY(9, 32);

	drawTm(DS1307_DATE, font_ks0066_ru_24);
	writeString((uint8_t*)"\x7F.\x7F");
	drawTm(DS1307_MONTH, font_ks0066_ru_24);
	writeString((uint8_t*)"\x7F.\x7F");
	if (getEtm() == DS1307_YEAR)
		gdLoadFont(font_ks0066_ru_24, 0, FONT_DIR_0);
	writeString((uint8_t*)"20");
	writeString((uint8_t*)"\x7F");
	drawTm(DS1307_YEAR, font_ks0066_ru_24);

	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);
	gdSetXY(32, 56);
#endif
	writeStringEeprom(txtLabels[LABEL_SUNDAY + (getTime(DS1307_WDAY) - 1) % 7]);

	return;
}

void showAlarm(void)
{
#ifdef KS0066
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"showAlarm");
#else
	uint8_t i, j;
	uint8_t *label;
	uint8_t ch;

	gdSetXY(4, 0);

	drawAm(DS1307_A0_HOUR, font_digits_32);
	writeString((uint8_t*)"\x7F:\x7F");
	drawAm(DS1307_A0_MIN, font_digits_32);

	/* Draw input icon selection rectangle */
	if (getEam() == DS1307_A0_INPUT) {
		gdDrawRect(96, 0, 32, 32, 1);
		gdDrawRect(97, 1, 30, 30, 1);
	} else {
		gdDrawRect(96, 0, 32, 32, 0);
		gdDrawRect(97, 1, 30, 30, 0);
	}

	gdSetXY(100, 4);
	/* Check that input number less than CHAN_CNT */
	i = getAlarm(DS1307_A0_INPUT);
	if (i >= sndInputCnt())
		i = 0;
	gdWriteIcon24(sndParAddr(MODE_SND_GAIN0 + i)->icon);

	/* Draw weekdays selection rectangle */
	if (getEam() == DS1307_A0_WDAY) {
		gdDrawRect(0, 34, 128, 30, 1);
		gdDrawRect(1, 35, 126, 28, 1);
	} else {
		gdDrawRect(0, 34, 128, 30, 0);
		gdDrawRect(1, 35, 126, 28, 0);
	}

	/* Draw weekdays */
	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);
	for (i = 0; i < 7; i++) {
		gdSetXY(5 + 18 * i, 38);
		j = 0;
		label = txtLabels[LABEL_SUNDAY + (i + 1) % 7];
		do {
			ch = eeprom_read_byte(&label[j++]);
		} while (ch == ' ');
		gdWriteChar(ch);
		gdWriteChar(0x7F);
		gdWriteChar(eeprom_read_byte(&label[j++]));

		gdDrawRect(3 + 18 * i, 47, 14, 14, 1);
		if (getAlarm(DS1307_A0_WDAY) & (0x40 >> i))
			gdDrawFilledRect(5 + 18 * i, 49, 10, 10, 1);
		else
			gdDrawFilledRect(5 + 18 * i, 49, 10, 10, 0);
	}
#endif
	return;
}

void showTimer(int16_t timer)
{
#ifdef KS0066
	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"showTimer");
#else
	uint8_t x, xbase;
	uint8_t y, ybase;

	volatile uint8_t *buf = getSpData(fallSpeed);

	gdSetXY(4, 0);

	gdLoadFont(font_digits_32, 1, FONT_DIR_0);
	if (timer >= 0) {
		writeNum(timer / 3600, 2, '0', 10);
		writeString((uint8_t*)"\x7F:\x7F");
		writeNum(timer / 60 % 60, 2, '0', 10);
		writeString((uint8_t*)"\x7F:\x7F");
		writeNum(timer % 60, 2, '0', 10);
	} else {
		writeString((uint8_t*)"--");
		writeString((uint8_t*)"\x7F:\x7F");
		writeString((uint8_t*)"--");
		writeString((uint8_t*)"\x7F:\x7F");
		writeString((uint8_t*)"--");
	}
	gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);

	for (y = 0; y < GD_SIZE_Y / 2; y++) {
		for (x = 0; x < GD_SIZE_X / 4; x++) {
			xbase = x << 2;
			ybase = 63 - y;
			if (buf[x] + buf[x + 32] >= y * 2) {
				gdDrawPixel(xbase + 0, ybase, 1);
				gdDrawPixel(xbase + 1, ybase, 1);
				gdDrawPixel(xbase + 2, ybase, 1);
			} else {
				gdDrawPixel(xbase + 0, ybase, 0);
				gdDrawPixel(xbase + 1, ybase, 0);
				gdDrawPixel(xbase + 2, ybase, 0);
			}
		}
	}
#endif
	return;
}

void switchSpMode(void)
{
	if (++spMode >= SP_MODE_END)
		spMode = SP_MODE_STEREO;

	return;
}

void switchFallSpeed(void)
{
	fallSpeed++;
	if (fallSpeed > FALL_SPEED_FAST)
		fallSpeed = FALL_SPEED_LOW;

	return;
}

void showSpectrum(void)
{
	volatile uint8_t *buf = getSpData(fallSpeed);
	uint16_t left, right;

#ifdef KS0066
	uint8_t i, data;

	switch (spMode) {
	case SP_MODE_STEREO:
		if (userSybmols != LCD_LEVELS)
			lcdGenLevels();
		ks0066SetXY(0, 0);
		for (i = 0; i < KS0066_SCREEN_WIDTH; i++) {
			data = buf[i] >> 2;
			if (data >= 7)
				data = 0xFF;
			ks0066WriteData(data);
		}
		ks0066SetXY(0, 1);
		for (i = 0; i < KS0066_SCREEN_WIDTH; i++) {
			data = buf[FFT_SIZE / 2 + i] >> 2;
			if (data >= 7)
				data = 0xFF;
			ks0066WriteData(data);
		}
		break;
	case SP_MODE_METER:
		if (userSybmols != LCD_BAR)
			lcdGenBar(userAddSym);

		left = 0;
		right = 0;
		for (i = 0; i < FFT_SIZE / 2; i++) {
			left += buf[i];
			right += buf[FFT_SIZE / 2 + i];
		}
		left >>= 4;
		right >>= 4;

		ks0066SetXY(0, 0);
		ks0066WriteData(eeprom_read_byte(txtLabels[LABEL_LEFT_CHANNEL]));
		for (i = 0; i < KS0066_SCREEN_WIDTH - 1; i++) {
			if (left / 3 > i) {
				ks0066WriteData(0x03);
			} else {
				if (left / 3 < i) {
					ks0066WriteData(0x00);
				} else {
					ks0066WriteData(left % 3);
				}
			}
		}
		ks0066SetXY(0, 1);
		ks0066WriteData(eeprom_read_byte(txtLabels[LABEL_RIGHT_CHANNEL]));
		for (i = 0; i < KS0066_SCREEN_WIDTH - 1; i++) {
			if (right / 3 > i) {
				ks0066WriteData(0x03);
			} else {
				if (right / 3 < i) {
					ks0066WriteData(0x00);
				} else {
					ks0066WriteData(right % 3);
				}
			}
		}
		break;
	default:
		if (userSybmols != LCD_LEVELS)
			lcdGenLevels();
		for (i = 0; i < KS0066_SCREEN_WIDTH; i++) {
			data = buf[i];
			data += buf[FFT_SIZE / 2 + i];
			data >>= 2;
			ks0066SetXY(i, 0);
			if (data < 8)
				ks0066WriteData(' ');
			else if (data < 15)
				ks0066WriteData(data - 8);
			else
				ks0066WriteData(0xFF);
			ks0066SetXY(i, 1);
			if (data < 7)
				ks0066WriteData(data);
			else
				ks0066WriteData(0xFF);
		}
		break;
	}
#else
	uint8_t x, xbase;
	uint8_t y, ybase;

	switch (spMode) {
	case SP_MODE_STEREO:
		for (y = 0; y < GD_SIZE_Y / 2; y++) {
			for (x = 0; x < GD_SIZE_X / 4; x++) {
				xbase = x << 2;
				ybase = 31 - y;
				if (buf[x] >= y) {
					gdDrawPixel(xbase + 0, ybase, 1);
					gdDrawPixel(xbase + 1, ybase, 1);
					gdDrawPixel(xbase + 2, ybase, 1);
				} else {
					gdDrawPixel(xbase + 0, ybase, 0);
					gdDrawPixel(xbase + 1, ybase, 0);
					gdDrawPixel(xbase + 2, ybase, 0);
				}
				ybase = 63 - y;
				if (buf[x + 32] >= y) {
					gdDrawPixel(xbase + 0, ybase, 1);
					gdDrawPixel(xbase + 1, ybase, 1);
					gdDrawPixel(xbase + 2, ybase, 1);
				} else {
					gdDrawPixel(xbase + 0, ybase, 0);
					gdDrawPixel(xbase + 1, ybase, 0);
					gdDrawPixel(xbase + 2, ybase, 0);
				}
			}
		}
		break;
	case SP_MODE_METER:
		gdLoadFont(font_ks0066_ru_08, 1, FONT_DIR_0);
		gdSetXY(0, 0);
		writeStringEeprom(txtLabels[LABEL_LEFT_CHANNEL]);
		gdSetXY(0, 36);
		writeStringEeprom(txtLabels[LABEL_RIGHT_CHANNEL]);
		left = 0;
		right = 0;
		for (x = 0; x < GD_SIZE_X / 4; x++) {
			left += buf[x];
			right += buf[x + 32];
		}
		left >>= 3;
		right >>= 3;
		for (x = 0; x < GD_SIZE_X; x++) {
			if (x % 3 != 2) {
				for (y = 12; y < 27; y++) {
					if (x < left || y == 19) {
						gdDrawPixel(x, y, 1);
					} else {
						gdDrawPixel(x, y, 0);
					}
				}
				for (y = 48; y < 63; y++) {
					if (x < right || y == 55) {
						gdDrawPixel(x, y, 1);
					} else {
						gdDrawPixel(x, y, 0);
					}
				}
			}
		}
		break;
	default:
		for (y = 0; y < GD_SIZE_Y; y++) {
			for (x = 0; x < GD_SIZE_X / 4; x++) {
				xbase = x << 2;
				ybase = 63 - y;
				if (buf[x] + buf[x + 32] >= y) {
					gdDrawPixel(xbase + 0, ybase, 1);
					gdDrawPixel(xbase + 1, ybase, 1);
					gdDrawPixel(xbase + 2, ybase, 1);
				} else {
					gdDrawPixel(xbase + 0, ybase, 0);
					gdDrawPixel(xbase + 1, ybase, 0);
					gdDrawPixel(xbase + 2, ybase, 0);
				}
			}
		}
		break;
	}
#endif
	return;
}

void setWorkBrightness(void)
{
#ifdef KS0066
	ks0066SetBrightness(brWork);
#if defined(KS0066_WIRE_PCF8574)
	pcf8574SetBacklight(brWork);
#endif
#else
	gdSetBrightness(brWork);
#endif
	return;
}

void setStbyBrightness(void)
{
#ifdef KS0066
	ks0066SetBrightness(brStby);
#if defined(KS0066_WIRE_PCF8574)
	pcf8574SetBacklight(KS0066_BCKL_OFF);
#endif
#else
	gdSetBrightness(brStby);
#endif
	return;
}

void displayPowerOff(void)
{
	eeprom_update_byte(eepromBrStby, brStby);
	eeprom_update_byte(eepromBrWork, brWork);
	eeprom_update_byte(eepromSpMode, spMode);
	eeprom_update_byte(eepromDisplay, defDisplay);
	eeprom_update_byte(eepromFallSpeed, fallSpeed);

	return;
}

