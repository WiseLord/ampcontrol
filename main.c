#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "adc.h"
#include "input.h"
#include "rc5.h"
#include "i2c.h"

#include "tda7313.h"
#include "display.h"

#define LANG_EN		0
#define LANG_RU		1

static uint8_t *txtLabels[LABELS_COUNT];	/* Array with text label pointers */
static uint8_t lang = LANG_EN;

/* Handle leaving standby mode */
static void powerOn(void)
{
	setWorkBrightness();
	unmuteVolume();

	return;
}

/* Handle entering standby mode */
static void powerOff(void)
{
	muteVolume();
	setStbyBrightness();
	stopEditTime();

	saveAudioParams();
	saveDisplayParams();

	return;
}

/* Load text labels from EEPROM */
static void loadLabels(uint8_t **txtLabels)
{
	uint8_t i;
	uint8_t *addr;

	if (lang == LANG_RU)
		addr = labelsAddrRu;
	else
		addr = labelsAddrEn;

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

	return;
}

static void changeLang(void)
{
	switch (lang) {
	case LANG_EN:
		lang = LANG_RU;
		break;
	default:
		lang = LANG_EN;
		break;
	}

	eeprom_update_byte(eepromLang, lang);

	loadLabels(txtLabels);
	loadAudioParams(txtLabels);
	ks0066Clear();

	return;
}

/* Hardware initialization */
static void hwInit(void)
{
	lang = eeprom_read_byte(eepromLang);
	loadLabels(txtLabels);				/* Load text labels from EEPROM */

	I2CInit();							/* I2C bus */
	ks0066Init();						/* Display */
	rc5Init();							/* IR Remote control */
	adcInit();							/* Analog-to-digital converter */
	inputInit();						/* Buttons/encoder polling */

	DDR(STMU_MUTE) |= STMU_MUTE_LINE;	/* Mute port */
	DDR(BCKL) |= BCKL_LINE;

	sei();								/* Gloabl interrupt enable */

	muteVolume();

	return;
}

int main(void)
{
	hwInit();

	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = dispMode;

	uint8_t curSndParam = SND_VOLUME;

	int8_t encCnt = 0;
	uint8_t cmd = CMD_EMPTY;
	uint16_t rc5Buf = RC5_BUF_EMPTY;
	uint16_t rc5BufPrev = RC5_BUF_EMPTY;

	loadAudioParams(txtLabels);
	loadDispParams();
	setStbyBrightness();

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

		/* Don't handle any command in standby mode except power on and test */
		if (dispMode == MODE_STANDBY) {
			if (cmd != CMD_BTN_1 && cmd != CMD_RC5_STBY && cmd != CMD_BTN_12_LONG && cmd != CMD_BTN_14_LONG)
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
			case MODE_GAIN0:
				nextChan();
				ks0066Clear();
			default:
				curSndParam = SND_GAIN0 + getChan();
				dispMode = MODE_GAIN0;
				break;
			}
			setDisplayTime(DISPLAY_TIME_GAIN);
			break;
		case CMD_BTN_3:
		case CMD_RC5_TIME:
			switch (dispMode) {
			case MODE_TIME:
			case MODE_TIME_EDIT:
				editTime();
				dispMode = MODE_TIME_EDIT;
				setDisplayTime(DISPLAY_TIME_TIME_EDIT);
				if (!isETM())
					setDisplayTime(DISPLAY_TIME_TIME);
				break;
			default:
				stopEditTime();
				dispMode = MODE_TIME;
				setDisplayTime(DISPLAY_TIME_TIME);
				break;
			}
			break;
		case CMD_BTN_4:
		case CMD_RC5_MUTE:
			ks0066Clear();
			switchMute();
			dispMode = MODE_MUTE;
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case CMD_BTN_5:
		case CMD_RC5_MENU:
			if (dispMode < MODE_FRONTREAR) {
				curSndParam++;
				dispMode++;
			} else {
				curSndParam = SND_VOLUME;
				dispMode = MODE_VOLUME;
			}
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case CMD_BTN_1_LONG:
		case CMD_RC5_BACKLIGHT:
			dispMode = MODE_BR;
			setDisplayTime(DISPLAY_TIME_BR);
			break;
		case CMD_BTN_4_LONG:
		case CMD_RC5_LOUDNESS:
			ks0066Clear();
			switchLoudness();
			dispMode = MODE_LOUDNESS;
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case CMD_BTN_12_LONG:
			if (dispMode == MODE_STANDBY) {
				dispMode = MODE_TEST;
				setDisplayTime(DISPLAY_TIME_TEST);
			}
			break;
		case CMD_BTN_14_LONG:
			if (dispMode == MODE_STANDBY)
				changeLang();
			break;
		case CMD_RC5_INPUT_0:
		case CMD_RC5_INPUT_1:
		case CMD_RC5_INPUT_2:
			setChan(cmd - CMD_RC5_INPUT_0);
			ks0066Clear();
			curSndParam = SND_GAIN0 + getChan();
			dispMode = MODE_GAIN0;
			setDisplayTime(DISPLAY_TIME_GAIN);
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
			case MODE_BR:
				changeBrWork(encCnt);
				setDisplayTime(DISPLAY_TIME_BR);
				break;
			case MODE_SPECTRUM:
			case MODE_TIME:
				curSndParam = SND_VOLUME;
				dispMode = MODE_VOLUME;
			default:
				changeParam(curSndParam, encCnt);
				setDisplayTime(DISPLAY_TIME_GAIN);
				break;
			}
		}

		/* Exit to default mode */
		if (getDisplayTime() == 0) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEST:
				setStbyBrightness();
				break;
			default:
				dispMode = MODE_SPECTRUM;
				break;
			}
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev && dispMode != MODE_TIME_EDIT)
			ks0066Clear();

		/* Stop time edit */
		if (dispMode != MODE_TIME_EDIT) {
			stopEditTime();
			ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
		}

		/* Show things */
		switch (dispMode) {
		case MODE_STANDBY:
			showTime(txtLabels);
			if (dispModePrev == MODE_TEST)
				setStbyBrightness();
			break;
		case MODE_TEST:
			showRC5Info(rc5Buf);
			setWorkBrightness();
			if (rc5Buf != rc5BufPrev)
				setDisplayTime(DISPLAY_TIME_TEST);
			break;
		case MODE_SPECTRUM:
			showSpectrum(getSpData());
			_delay_ms(20);
			break;
		case MODE_MUTE:
			showMute(getMute(), txtLabels);
			break;
		case MODE_LOUDNESS:
			showLoudness(getLoudness(), txtLabels);
			break;
		case MODE_TIME:
		case MODE_TIME_EDIT:
			showTime(txtLabels);
			break;
		case MODE_BR:
			showBrWork(txtLabels);
			break;
		default:
			showSndParam(curSndParam, txtLabels);
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;
		/* Save current RC5 raw buf */
		rc5BufPrev = rc5Buf;
	}

	return 0;
}
