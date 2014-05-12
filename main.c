#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"
#include "rc5.h"
#include "i2c.h"
#include "audio.h"
#include "ds1307.h"
#include "tea5767.h"
#include "display.h"

#define DISPLAY_TIME_TEST		20
#define DISPLAY_TIME_GAIN		3
#define DISPLAY_TIME_TIME		3
#define DISPLAY_TIME_TIME_EDIT	10
#define DISPLAY_TIME_FM_RADIO	20
#define DISPLAY_TIME_CHAN		2
#define DISPLAY_TIME_AUDIO		3
#define DISPLAY_TIME_TEST		20

#define BACKLIGHT_ON			0
#define BACKLIGHT_OFF			1

#define EEPROM_SIZE				0x200

#define FM_COUNT				64

enum {
	MODE_STANDBY,
	MODE_SPECTRUM,
	MODE_FM_RADIO,
	MODE_VOLUME,
	MODE_BASS,
	MODE_MIDDLE,
	MODE_TREBLE,
	MODE_PREAMP,
	MODE_GAIN,
	MODE_BALANCE,
	MODE_TIME,
	MODE_TIME_EDIT,
	MODE_MUTE,
	MODE_LOUDNESS,
	MODE_TEST
};

uint8_t spMode;
uint8_t backlight;

uint8_t rc5DeviceAddr;
uint8_t rcCode[RC5_CMD_COUNT];	/* Array with rc5 commands */

uint8_t *txtLabels[LABELS_COUNT];

uint16_t freqFM;

void switchSpMode()
{
	spMode = !spMode;
	return;
}

void loadLabels()
{
	uint8_t i;
	uint8_t *addr;

	addr = labelsAddr;
	i = 0;

	while (i < LABELS_COUNT && addr < (uint8_t*)EEPROM_SIZE) {
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

}

uint8_t stationNum(uint16_t freq)
{
	uint8_t i;

	for (i = 0; i < FM_COUNT; i++)
		if (eeprom_read_word(eepromStations + i) == freq)
			return i + 1;

	return 0;
}

void scanStoredFreq(uint16_t freq, uint8_t direction)
{
	uint8_t i;
	uint16_t freqCell;
	uint16_t freqFound = freq;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell != 0xFFFF) {
			if (direction) {
				if (freqCell > freq) {
					freqFound = freqCell;
					break;
				}
			} else {
				if (freqCell < freq) {
					freqFound = freqCell;
				} else {
					break;
				}
			}
		}
	}

	tea5767SetFreq(freqFound);
}

void loadStation(uint8_t num)
{
	uint16_t freqCell = eeprom_read_word(eepromStations + num);

	if (freqCell != 0xFFFF)
		tea5767SetFreq(freqCell);
}

void storeStation(uint16_t freq)
{
	uint8_t i, j;
	uint16_t freqCell;

	for (i = 0; i < FM_COUNT; i++) {
		freqCell = eeprom_read_word(eepromStations + i);
		if (freqCell < freq)
			continue;
		if (freqCell == freq) {
			for (j = i; j < FM_COUNT; j++) {
				if (i == FM_COUNT - 1)
					freqCell = 0xFFFF;
				else
					freqCell = eeprom_read_word(eepromStations + j + 1);
				eeprom_write_word(eepromStations + j, freqCell);
			}
			break;
		} else {
			for (j = i; j < FM_COUNT; j++) {
				freqCell = eeprom_read_word(eepromStations + j);
				eeprom_write_word(eepromStations + j, freq);
				freq = freqCell;
			}
			break;
		}
	}
}

void hwInit(void)	/* Hardware initialization */
{
	uint8_t i;

	/* Load RC5 device address and commands from eeprom */
	rc5DeviceAddr = eeprom_read_byte(eepromRC5Addr);
	for (i = 0; i < RC5_CMD_COUNT; i++) {
		rcCode[i] = eeprom_read_byte(eepromRC5Cmd + i);
	}

	loadLabels();

	gdInit();							/* Graphic display */
	gdLoadFont(font_ks0066_ru_08, 1);
	rc5Init();							/* IR Remote control */
	adcInit();							/* Analog-to-digital converter */
	btnInit(rcCode, rc5DeviceAddr);		/* Buttons/encoder polling */
	I2CInit();							/* I2C bus */
	tea5767Init();

	freqFM = eeprom_read_word(eepromFMFreq);

	etm = NOEDIT;

	SMF_DDR |= (STDBY | FAN);
	SMF_PORT &= ~(STDBY | MUTE | FAN);

	sei();
	return;
}

void setBacklight(int8_t backlight)
{
	if (backlight)
		GD_BACKLIGHT_PORT |= GD_BCKL;
	else
		GD_BACKLIGHT_PORT &= ~GD_BCKL;
}

void showParam(sndParam *param)
{
	uint8_t mult = 8;

#ifndef TDA7439
	if (param->label == txtLabels[LABEL_VOLUME]
	 || param->label == txtLabels[LABEL_PREAMP]
	 || param->label == txtLabels[LABEL_BALANCE])
	{
		mult = 10;
	}
	if (param->label == txtLabels[LABEL_GAIN_0]
	 || param->label == txtLabels[LABEL_GAIN_1]
	 || param->label == txtLabels[LABEL_GAIN_2]
	 || param->label == txtLabels[LABEL_GAIN_3])
	{
		mult = 15;
	}
#endif
	showBar(param->min, param->max, param->value);
	showParValue(((int16_t)(param->value) * param->step * mult + 4) >> 3);
	showParLabel(param->label, txtLabels);
}

void saveParams(void)
{
	saveAudioParams();
	eeprom_write_byte(eepromBCKL, backlight);
	eeprom_write_byte(eepromSpMode, spMode);
	eeprom_write_word(eepromFMFreq, freqFM);
}

void switchBacklight(void)
{
	backlight = !backlight;
	setBacklight(backlight);
}

void powerOn(void)
{
	SMF_PORT |= STDBY;
	_delay_ms(50);
	SMF_DDR |= MUTE;
	SMF_PORT |= MUTE;
	SMF_PORT |= FAN;
	setBacklight(backlight);
	unmuteVolume();
}

void powerOff(void)
{
	SMF_DDR &= ~MUTE;
	SMF_PORT &= ~MUTE;
	_delay_ms(50);
	SMF_PORT &= ~STDBY;
	SMF_PORT &= ~FAN;
	GD_BACKLIGHT_PORT &= ~GD_BCKL;
	etm = NOEDIT;
	muteVolume();
	saveParams();
}

int main(void)
{
	hwInit();

	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = MODE_STANDBY;

	uint8_t *spBuf;
	sndParam *curSndParam = &volume;

	int8_t encCnt = 0;
	uint8_t cmd = CMD_EMPTY;
	uint16_t rc5Buf = RC5_BUF_EMPTY;
	uint16_t rc5BufPrev = RC5_BUF_EMPTY;

	spMode  = eeprom_read_byte(eepromSpMode);

	loadParams(txtLabels);
	backlight = eeprom_read_byte(eepromBCKL);
	muteVolume();

	uint8_t bufFM[5];

	tea5767SetFreq(freqFM);

	while (1) {
		encCnt = getEncoder();
		cmd = getBtnCmd();
		rc5Buf = getRC5Buf();

		/* Don't handle any command in test mode */
		if (dispMode == MODE_TEST) {
			if (cmd != CMD_EMPTY)
				setDisplayTime(DISPLAY_TIME_TEST);
			cmd = CMD_EMPTY;
		}

		/* Don't handle any command in standby mode except power on */
		if (dispMode == MODE_STANDBY) {
			if (cmd != CMD_BTN_1 && cmd != CMD_RC5_STBY &&
			    cmd != CMD_BTN_TESTMODE)
				cmd = CMD_EMPTY;
		}

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
		case CMD_RC5_STBY:
			switch (dispMode) {
			case MODE_STANDBY:
				powerOn();
				dispMode = MODE_SPECTRUM;
				break;
			default:
				powerOff();
				dispMode = MODE_STANDBY;
				break;
			}
			break;
		case CMD_BTN_2:
		case CMD_RC5_NEXT_INPUT:
			switch (dispMode) {
			case MODE_GAIN:
				nextChan();
				gdFill(0x00);
			default:
				curSndParam = &gain[chan];
				dispMode = MODE_GAIN;
				setDisplayTime(DISPLAY_TIME_GAIN);
				break;
			}
			break;
		case CMD_BTN_3:
		case CMD_RC5_TIME:
			switch (dispMode) {
			case MODE_FM_RADIO:
				if (cmd != CMD_RC5_TIME) {
					tea5767SetFreq(freqFM - 10);
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				}
			case MODE_TIME:
			case MODE_TIME_EDIT:
				if (cmd != CMD_RC5_CHAN_DOWN) {
					editTime();
					dispMode = MODE_TIME_EDIT;
					setDisplayTime(DISPLAY_TIME_TIME_EDIT);
					if (!isETM())
						setDisplayTime(DISPLAY_TIME_TIME);
					break;
				}
			default:
				if (cmd != CMD_RC5_CHAN_DOWN) {
					stopEditTime();
					dispMode = MODE_TIME;
					setDisplayTime(DISPLAY_TIME_TIME);
					break;
				}
				break;
			}
			break;
		case CMD_BTN_4:
		case CMD_RC5_MUTE:
			switch (dispMode) {
			case MODE_FM_RADIO:
				if (cmd != CMD_RC5_MUTE) {
					tea5767SetFreq(freqFM + 10);
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				}
			default:
				if (cmd != CMD_RC5_CHAN_UP) {
					switchMute();
					dispMode = MODE_MUTE;
					setDisplayTime(DISPLAY_TIME_CHAN);
					break;
				}
			}
			break;
		case CMD_BTN_5:
		case CMD_RC5_MENU:
			switch (dispMode) {
			case MODE_VOLUME:
				curSndParam = &bass;
				dispMode = MODE_BASS;
				break;
			case MODE_BASS:
#ifdef TDA7439
				curSndParam = &middle;
				dispMode = MODE_MIDDLE;
				break;
			case MODE_MIDDLE:
#endif
				curSndParam = &treble;
				dispMode = MODE_TREBLE;
				break;
			case MODE_TREBLE:
				curSndParam = &preamp;
				dispMode = MODE_PREAMP;
				break;
			case MODE_PREAMP:
				curSndParam = &balance;
				dispMode = MODE_BALANCE;
				break;
			default:
				curSndParam = &volume;
				dispMode = MODE_VOLUME;
				break;
			}
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case CMD_BTN_1_LONG:
		case CMD_RC5_BACKLIGHT:
			switchBacklight();
			break;
		case CMD_BTN_2_LONG:
			switch (dispMode) {
			default:
				if (chan == 0) {
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				}
			}
			break;
		case CMD_BTN_3_LONG:
		case CMD_BTN_4_LONG:
		case CMD_BTN_5_LONG:
		case CMD_RC5_FM_STORE:
			if (dispMode == MODE_FM_RADIO) {
				if (cmd == CMD_BTN_3_LONG)
					tea5767Search(freqFM, bufFM, SEARCH_DOWN);
				else if (cmd == CMD_BTN_4_LONG)
					tea5767Search(freqFM, bufFM, SEARCH_UP);
				else
					storeStation(freqFM);
				setDisplayTime(DISPLAY_TIME_FM_RADIO);
			}
			break;
		case CMD_BTN_TESTMODE:
			switch (dispMode) {
			case MODE_STANDBY:
				dispMode = MODE_TEST;
				setDisplayTime(DISPLAY_TIME_TEST);
				break;
			}
			break;
#ifdef TDA7313
		case CMD_RC5_LOUDNESS:
			switchLoudness();
			dispMode = MODE_LOUDNESS;
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
#endif
		case CMD_RC5_INPUT_0:
		case CMD_RC5_INPUT_1:
		case CMD_RC5_INPUT_2:
#ifndef TDA7313
		case CMD_RC5_INPUT_3:
#endif
			switch (dispMode) {
			default:
				if (dispMode != MODE_FM_RADIO &&
				    chan == 0 && cmd == CMD_RC5_INPUT_0 + chan) {
					curSndParam = &volume;
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
				} else {
					setChan(cmd - CMD_RC5_INPUT_0);
					gdFill(0x00);
					curSndParam = &gain[chan];
					dispMode = MODE_GAIN;
					setDisplayTime(DISPLAY_TIME_GAIN);
				}
				break;
			}
			break;
		case CMD_RC5_SP_MODE:
			switchSpMode();
			saveParams();
			dispMode = MODE_SPECTRUM;
			break;
		case CMD_RC5_FM_INC:
		case CMD_RC5_FM_DEC:
		case CMD_RC5_CHAN_UP:
		case CMD_RC5_CHAN_DOWN:
			if (chan == 0) {
				if (dispMode == MODE_FM_RADIO) {
					switch (cmd) {
					case CMD_RC5_FM_INC:
						tea5767SetFreq(freqFM + 10);
						break;
					case CMD_RC5_FM_DEC:
						tea5767SetFreq(freqFM - 10);
						break;
					case CMD_RC5_CHAN_UP:
						scanStoredFreq(freqFM, SEARCH_UP);
						break;
					case CMD_RC5_CHAN_DOWN:
						scanStoredFreq(freqFM, SEARCH_DOWN);
						break;
					}
				}
				dispMode = MODE_FM_RADIO;
				setDisplayTime(DISPLAY_TIME_FM_RADIO);
			}
			break;
		case CMD_RC5_1:
		case CMD_RC5_2:
		case CMD_RC5_3:
		case CMD_RC5_4:
		case CMD_RC5_5:
		case CMD_RC5_6:
		case CMD_RC5_7:
		case CMD_RC5_8:
		case CMD_RC5_9:
		case CMD_RC5_0:
			if (chan == 0) {
				loadStation(cmd - CMD_RC5_1);
				dispMode = MODE_FM_RADIO;
				setDisplayTime(DISPLAY_TIME_FM_RADIO);
			}
			break;
		}

		/* Emulate RC5 VOL_UP/VOL_DOWN as encoder actions */
		if (cmd == CMD_RC5_VOL_UP)
			encCnt++;
		if (cmd == CMD_RC5_VOL_DOWN)
			encCnt--;

		/* Handle encoder */
		if (encCnt) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEST:
				setDisplayTime(DISPLAY_TIME_TEST);
				break;
			case MODE_TIME_EDIT:
				changeTime(encCnt);
				setDisplayTime(DISPLAY_TIME_TIME_EDIT);
				break;
			case MODE_SPECTRUM:
			case MODE_TIME:
			case MODE_FM_RADIO:
				curSndParam = &volume;
				dispMode = MODE_VOLUME;
			default:
				changeParam(curSndParam, encCnt);
				setDisplayTime(DISPLAY_TIME_GAIN);
				break;
			}
		}

		/* Exit to default mode and save params to EEPROM*/
		if (getDisplayTime() == 0) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEST:
				dispMode = MODE_STANDBY;
				break;
			default:
				if (dispModePrev != MODE_SPECTRUM) {
					gdSetXY(0, 0);
					saveParams();
				}
				dispMode = MODE_SPECTRUM;
				break;
			}
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			gdFill(0x00);

		/* Show things */
		switch (dispMode) {
		case MODE_STANDBY:
			showTime(txtLabels);
			if (dispModePrev == MODE_TEST)
				setBacklight(0);
			break;
		case MODE_TEST:
			showRC5Info(rc5Buf);
			setBacklight(backlight);
			if (rc5Buf != rc5BufPrev)
				setDisplayTime(DISPLAY_TIME_TEST);
			break;
		case MODE_SPECTRUM:
			spBuf = getSpData();
			drawSpectrum(spBuf, spMode);
			break;
		case MODE_FM_RADIO:
			tea5767ReadStatus(bufFM);
			freqFM = tea5767FreqAvail(bufFM);
			showRadio(bufFM, stationNum(freqFM));
			if (TEA5767_BUF_READY(bufFM))
				fineTune(&freqFM, bufFM);
			break;
		case MODE_MUTE:
			showBoolParam(mute, txtLabels[LABEL_MUTE], txtLabels);
			break;
		case MODE_LOUDNESS:
			showBoolParam(!loud, txtLabels[LABEL_MIDDLE], txtLabels);
			break;
		case MODE_TIME:
		case MODE_TIME_EDIT:
			showTime(txtLabels);
			break;
		default:
			showParam(curSndParam);
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;
		/* Save current RC5 raw buf */
		rc5BufPrev = rc5Buf;
	}

	return 0;
}
