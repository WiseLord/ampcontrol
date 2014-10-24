#include "display.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "input.h"
#include "adc.h"

static int8_t brStby;							/* Brightness in standby mode */
static int8_t brWork;							/* Brightness in working mode */

static uint8_t strbuf[STR_BUFSIZE + 1];			/* String buffer */

static const uint8_t timeCurPos[] PROGMEM = {
	7, 4, 1, 0, 12, 15, 79
};

static const uint8_t barSymbols[] PROGMEM = {
	0x00, 0x10, 0x14, 0x15, 0x05, 0x01,
};

static const uint8_t cyr_P[]  PROGMEM = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00};
static const uint8_t cyr_D[]  PROGMEM = {0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x1F, 0x11};
static const uint8_t cyr_L[]  PROGMEM = {0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x11, 0x00};
static const uint8_t cyr_I[]  PROGMEM = {0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00};
static const uint8_t cyr_CH[] PROGMEM = {0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00};
static const uint8_t cyr_G[]  PROGMEM = {0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00};
static const uint8_t cyr_YA[] PROGMEM = {0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00};
static const uint8_t cyr_TS[] PROGMEM = {0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x1F, 0x01};
static const uint8_t cyr_U[]  PROGMEM = {0x11, 0x11, 0x11, 0x0F, 0x01, 0x11, 0x0E, 0x00};
static const uint8_t cyr_B[]  PROGMEM = {0x1F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00};
static const uint8_t cyr_F[]  PROGMEM = {0x04, 0x0E, 0x15, 0x15, 0x15, 0x0E, 0x04, 0x00};
static const uint8_t cyr_YU[] PROGMEM = {0x12, 0x15, 0x15, 0x1D, 0x15, 0x15, 0x12, 0x00};
static const uint8_t cyr_SH[] PROGMEM = {0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x1F, 0x00};

static uint8_t rc5CmdInd;
static uint8_t rc5Cmd;
static uint8_t rc5Addr;

static const uint8_t labelSTBY[]   PROGMEM = "Standby";
static const uint8_t labelMUTE[]   PROGMEM = "Mute";
static const uint8_t labelMENU[]   PROGMEM = "Menu";
static const uint8_t labelVOLUP[]  PROGMEM = "Volume up";
static const uint8_t labelVOLDN[]  PROGMEM = "Volume down";
static const uint8_t labelIN_0[]   PROGMEM = "Input 1";
static const uint8_t labelIN_1[]   PROGMEM = "Input 2";
static const uint8_t labelIN_2[]   PROGMEM = "Input 3";
static const uint8_t labelNEXTIN[] PROGMEM = "Next input";
static const uint8_t labelTIME[]   PROGMEM = "Time";
static const uint8_t labelBCKL[]   PROGMEM = "Backlight";
static const uint8_t labelBRIGHT[] PROGMEM = "Loudness";

static const uint8_t *const labelsRC5[] PROGMEM = {
	labelSTBY,
	labelMUTE,
	labelMENU,
	labelVOLUP,
	labelVOLDN,
	labelIN_0,
	labelIN_1,
	labelIN_2,
	labelNEXTIN,
	labelTIME,
	labelBCKL,
	labelBRIGHT
};

static void writeStringEeprom(const uint8_t *string)
{
	uint8_t i = 0;

	for (i = 0; i < STR_BUFSIZE; i++)
		strbuf[i] = eeprom_read_byte(&string[i]);

	ks0066WriteString(strbuf);

	return;
}

static void writeStringPgm(const uint8_t *string)
{
	uint8_t i = 0;

	for (i = 0; i < STR_BUFSIZE; i++)
		strbuf[i] = pgm_read_byte(&string[i]);

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

	for (i = 0; i < 8 * sizeof(barSymbols); i++) {
			if ((i & 0x07) == 0x03) {
					ks0066WriteData(0x15);
			} else if ((i & 0x07) == 0x07) {
					ks0066WriteData(0x00);
			} else {
				ks0066WriteData(pgm_read_byte(&barSymbols[i>>3]));
			}
	}

	return;
}

static void lcdGenLetter(const uint8_t *letter)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
		ks0066WriteData(pgm_read_byte(&letter[i]));

	return;
}

static void lcdGenMuteLoudLabels(void)
{
	ks0066WriteCommand(KS0066_SET_CGRAM);

	lcdGenLetter(cyr_TS);
	lcdGenLetter(cyr_I);
	lcdGenLetter(cyr_YA);
	lcdGenLetter(cyr_P);
	lcdGenLetter(cyr_G);
	lcdGenLetter(cyr_L);
	lcdGenLetter(cyr_U);
	lcdGenLetter(cyr_SH);

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
	uint8_t numdiv;

	strbuf[2] = '\0';
	numdiv = value % 16;
	if (numdiv >= 10)
		numdiv += 7;
	strbuf[1] = numdiv + 0x30;
	numdiv = value / 16 % 16;
	strbuf[0] = numdiv + 0x30;

	return strbuf;
}

static void showBar(int8_t min, int8_t max, int8_t value)
{
	uint8_t i;

	ks0066SetXY(0, 1);

	if (min + max) {
		value = (int16_t)48 * (value - min) / (max - min);
		for (i = 0; i < 16; i++)
			if (value / 3 > i)
				ks0066WriteData(0x03);
			else
				if (value / 3 < i)
					ks0066WriteData(0x00);
				else
					ks0066WriteData(value % 3);
	} else {
		value = (int16_t)23 * value / max;
		if (value >= 0) {
			value++;
			for (i = 0; i < 7; i++)
				ks0066WriteData(0x00);
			ks0066WriteData(0x05);
			for (i = 0; i < 8; i++)
				if (value / 3 > i)
					ks0066WriteData(0x03);
				else
					if (value / 3 < i)
						ks0066WriteData(0x00);
					else
						ks0066WriteData(value % 3);
		} else {
			value += 23;
			for (i = 0; i < 8; i++)
				if (value / 3 > i)
					ks0066WriteData(0x00);
				else
					if (value / 3 < i)
						ks0066WriteData(0x03);
					else
						ks0066WriteData(value % 3 + 3);
			ks0066WriteData(0x01);
			for (i = 0; i < 7; i++)
				ks0066WriteData(0x00);
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

void showRC5Info()
{
	uint16_t rc5Buf = getRC5Buf();

	rc5Addr = (rc5Buf & 0x07C0)>>6;
	rc5Cmd = rc5Buf & 0x003F;

	ks0066SetXY(0, 0);
	writeStringPgm((const uint8_t *)pgm_read_word(&labelsRC5[rc5CmdInd]));

	ks0066SetXY(13, 0);
	ks0066WriteString((uint8_t*)"RC5");

	ks0066SetXY(0, 1);
	ks0066WriteString(mkHexString(rc5Cmd));
	ks0066WriteString((uint8_t*)"=>");
	ks0066WriteString(mkHexString(eeprom_read_byte(eepromRC5Cmd + rc5CmdInd)));
	ks0066SetXY(10, 1);
	ks0066WriteString(mkHexString(rc5Addr));
	ks0066WriteString((uint8_t*)"=>");
	ks0066WriteString(mkHexString(eeprom_read_byte(eepromRC5Addr)));

	return;
}

void showMute(uint8_t value, uint8_t **txtLabels)
{
	lcdGenMuteLoudLabels();

	ks0066SetXY(0, 0);
	writeStringEeprom(txtLabels[LABEL_MUTE]);
	ks0066SetXY(1, 1);
	if (value)
		writeStringEeprom(txtLabels[LABEL_ON]);
	else
		writeStringEeprom(txtLabels[LABEL_OFF]);

	return;
}

void showLoudness(uint8_t value, uint8_t **txtLabels)
{
	lcdGenMuteLoudLabels();

	ks0066SetXY(0, 0);
	writeStringEeprom(txtLabels[LABEL_LOUDNESS]);
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
	lcdGenBar();
	lcdGenLetter(cyr_YA);
	showBar(DISP_MIN_BR, DISP_MAX_BR, brWork);
	ks0066SetXY(13, 0);
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
void showSndParam(uint8_t index, uint8_t **txtLabels)
{
	volatile sndParam *param = sndParAddr(index);

	lcdGenBar();
	switch (index) {
	case SND_VOLUME:
	case SND_FRONTREAR:
		lcdGenLetter(cyr_G);
		lcdGenLetter(cyr_F);
		break;
	case SND_BASS:
	case SND_TREBLE:
		lcdGenLetter(cyr_B);
		lcdGenLetter(cyr_CH);
		break;
	case SND_BALANCE:
		lcdGenLetter(cyr_B);
		lcdGenLetter(cyr_L);
		break;
	case SND_GAIN0:
	case SND_GAIN1:
		lcdGenLetter(cyr_P);
		lcdGenLetter(cyr_YU);
		break;
	case SND_GAIN2:
		lcdGenLetter(cyr_P);
		lcdGenLetter(cyr_L);
		break;
	}

	showBar(param->min, param->max, param->value);
	ks0066SetXY(11, 0);
	ks0066WriteString(mkNumString(((int16_t)(param->value) * param->step + 4) >> 3, 3, ' '));
	showParLabel(param->label, txtLabels);
	ks0066SetXY(14, 0);
	ks0066WriteString((uint8_t*)"dB");

	return;
}

void showTime(uint8_t **txtLabels)
{
	uint8_t i, flag;

	int8_t *time;
	static int8_t lastTime[7];

	time = readTime();

	flag = 0;
	for (i = 0; i < 7; i++) {
		if (lastTime[i] != time[i]) {
			flag = 1;
			break;
		}
	}

	if (flag) {
		if (lastTime[WEEK] != time[WEEK])
			ks0066Clear();

		ks0066SetXY(0, 0);
		ks0066WriteString(mkNumString(time[HOUR], 2, '0'));
		ks0066WriteData(':');
		ks0066WriteString(mkNumString(time[MIN], 2, '0'));
		ks0066WriteData(':');
		ks0066WriteString(mkNumString(time[SEC], 2, '0'));

		ks0066SetXY(11, 0);
		ks0066WriteString(mkNumString(time[DAY], 2, '0'));
		ks0066WriteData('.');
		ks0066WriteString(mkNumString(time[MONTH], 2, '0'));

		ks0066SetXY(12, 1);
		ks0066WriteString((uint8_t*)"20");
		ks0066WriteString(mkNumString(time[YEAR], 2, '0'));

		ks0066WriteCommand(KS0066_SET_CGRAM);
		if (time[WEEK] < 5) {
			lcdGenLetter(cyr_P);
			lcdGenLetter(cyr_D);
			lcdGenLetter(cyr_L);
			lcdGenLetter(cyr_I);
			lcdGenLetter(cyr_CH);
			lcdGenLetter(cyr_G);
		} else {
			lcdGenLetter(cyr_P);
			lcdGenLetter(cyr_YA);
			lcdGenLetter(cyr_I);
			lcdGenLetter(cyr_TS);
			lcdGenLetter(cyr_U);
			lcdGenLetter(cyr_B);
		}

		ks0066SetXY(0, 1);
		writeStringEeprom(txtLabels[LABEL_SUNDAY + time[WEEK] % 7]);

		if (getEtm() == NOEDIT) {
			ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
		} else {
			ks0066SetXY(pgm_read_byte(&timeCurPos[getEtm()]), 0);
			ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON | KS0066_CUR_BLINK_ON);
		}
	}

	for (i = 0; i < 7; i++)
		lastTime[i] = time[i];


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
