#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "adc.h"
#include "input.h"
#include "rc5.h"
#include "i2c.h"

#include "audio/audio.h"
#include "display.h"
#include "tuner/tuner.h"
#include "temp.h"

#include "actions.h"

uint8_t *txtLabels[LABELS_COUNT];	/* Array with text label pointers */

/* Load text labels from EEPROM */
static void loadLabels(uint8_t **txtLabels)
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

	return;
}

/* Hardware initialization */
static void hwInit(void)
{
	sei();								/* Gloabl interrupt enable */

	ds18x20SearchDevices();
	tempInit();							/* Init temperature control */

	inputInit();						/* Buttons/encoder polling */

	loadLabels(txtLabels);				/* Load text labels from EEPROM */

	gdInit();

	rc5Init();							/* IR Remote control */
	adcInit();							/* Analog-to-digital converter */
	I2CInit();							/* I2C bus */

	tunerInit();						/* Tuner */

	DDR(STMU_STBY) |= STMU_STBY_LINE;	/* Standby port */
	DDR(STMU_MUTE) |= STMU_MUTE_LINE;	/* Mute port */

	sndInit(txtLabels);					/* Load labels/icons/etc */
	loadDispParams();					/* Load display params */

	powerOff();

	return;
}

int main(void)
{
	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = dispMode;

	int8_t encCnt = 0;
	uint8_t cmd = CMD_EMPTY;

	int16_t stbyTimer = STBY_TIMER_OFF;

	hwInit();

	while (1) {
		encCnt = getEncoder();
		cmd = getBtnCmd();

		/* Emulate poweroff if standby timer expired */
		stbyTimer = getStbyTimer();
		if (stbyTimer == 0)
			cmd = CMD_RC5_STBY;

		ds18x20Process();
		tempControlProcess();

		/* Limit update time/alarm interval */
		if (getClockTimer() == 0) {
			readTime();
			readAlarm();

			if (dispMode == MODE_STANDBY)
				checkAlarm(&dispMode);

			setClockTimer(200);
		}

		/* Don't handle any command in test mode except BTN_5 and BTN_1_LONG */
		if (dispMode == MODE_TEST) {
			if (cmd != CMD_EMPTY)
				setDisplayTime(DISPLAY_TIME_TEST);
			if (cmd != CMD_BTN_5 && cmd != CMD_BTN_1_LONG)
				cmd = CMD_EMPTY;
		}
		/* Don't handle any command in temp mode except BTN_1_LONG */
		if (dispMode == MODE_TEMP) {
			if (cmd != CMD_EMPTY)
				setDisplayTime(DISPLAY_TIME_TEMP);
			if (cmd != CMD_BTN_1_LONG)
				cmd = CMD_EMPTY;
		}

		/* Don't handle commands in standby mode except STBY, TEST and TEMP */
		if (dispMode == MODE_STANDBY) {
			if (cmd != CMD_BTN_1 && cmd != CMD_RC5_STBY &&
				cmd != CMD_BTN_TEST && cmd != CMD_BTN_TEMP)
				cmd = CMD_EMPTY;
		}

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
			handleSwitchPower(&dispMode);
			break;
		case CMD_BTN_2:
			sndSetInput(sndGetInput() + 1);
			handleSetInput(&dispMode);
			break;
		case CMD_BTN_3:
			switch (dispMode) {
			case MODE_TIMER:
				handleChangeTimer(&dispMode, stbyTimer);
				break;
			case MODE_FM_TUNE:
			case MODE_FM_RADIO:
				handleChangeFM(&dispMode, SEARCH_DOWN);
				break;
			case MODE_TIME:
			case MODE_TIME_EDIT:
				handleEditTime(&dispMode);
				break;
			case MODE_ALARM:
			case MODE_ALARM_EDIT:
				handleEditAlarm(&dispMode);
				break;
			default:
				handleSwitchSpMode(&dispMode);
				break;
			}
			break;
		case CMD_BTN_4:
			switch (dispMode) {
			case MODE_FM_TUNE:
			case MODE_FM_RADIO:
				handleChangeFM(&dispMode, SEARCH_UP);
				break;
			default:
				handleSwitchMute(&dispMode);
				break;
			}
			break;
		case CMD_BTN_5:
			if (dispMode == MODE_TEST) {
				gdClear();
				nextRC5Cmd();
			} else {
				sndNextParam(&dispMode);
				setDisplayTime(DISPLAY_TIME_AUDIO);
			}
			break;
		case CMD_BTN_1_LONG:
			switch (dispMode) {
			case MODE_TEST:
			case MODE_TEMP:
				setDisplayTime(0);
				break;
			default:
				dispMode = MODE_BR;
				setDisplayTime(DISPLAY_TIME_BR);
				break;
			}
			break;
		case CMD_BTN_2_LONG:
			handleSetDefDisplay(&dispMode);
			break;
		case CMD_BTN_3_LONG:
			switch (dispMode) {
			case MODE_TIME:
			case MODE_TIME_EDIT:
				handleChangeTimer(&dispMode, stbyTimer);
				break;
			case MODE_TIMER:
				handleEditAlarm(&dispMode);
				break;
			case MODE_ALARM:
			case MODE_ALARM_EDIT:
				handleEditTime(&dispMode);
				break;
			default:
				handleEditTime(&dispMode);
				break;
			}
			break;
		case CMD_BTN_4_LONG:
			handleSwitchFmMode(&dispMode);
			break;
		case CMD_BTN_5_LONG:
			handleStoreStation(&dispMode);
			break;
		case CMD_BTN_TEST:
			switch (dispMode) {
			case MODE_STANDBY:
				dispMode = MODE_TEST;
				startTestMode();
				setDisplayTime(DISPLAY_TIME_TEST);
				break;
			}
			break;
		case CMD_BTN_TEMP:
			switch (dispMode) {
			case MODE_STANDBY:
				dispMode = MODE_TEMP;
				setDisplayTime(DISPLAY_TIME_TEMP);
				break;
			}
			break;
		case CMD_RC5_STBY:
			handleSwitchPower(&dispMode);
			break;
		case CMD_RC5_MUTE:
			handleSwitchMute(&dispMode);
			break;
		case CMD_RC5_MENU:
			sndNextParam(&dispMode);
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		/* CMD_VOL_UP and CMD_VOL_DOWN are processed below as encoder actions */
		case CMD_RC5_INPUT_0:
		case CMD_RC5_INPUT_1:
		case CMD_RC5_INPUT_2:
			sndSetInput(cmd - CMD_RC5_INPUT_0);
			handleSetInput(&dispMode);
			break;
		case CMD_RC5_INPUT_3:
			if (sndInputCnt() < 4) {
				sndSetLoudness(!sndGetLoudness());
				dispMode = MODE_LOUDNESS;
				setDisplayTime(DISPLAY_TIME_AUDIO);
			} else {
				sndSetInput(cmd - CMD_RC5_INPUT_0);
				handleSetInput (&dispMode);
			}
			break;
		case CMD_RC5_NEXT_INPUT:
			sndSetInput(sndGetInput() + 1);
			handleSetInput (&dispMode);
			break;
		case CMD_RC5_TIME:
			handleEditTime(&dispMode);
			break;
		case CMD_RC5_BACKLIGHT:
			dispMode = MODE_BR;
			setDisplayTime(DISPLAY_TIME_BR);
			break;
		case CMD_RC5_SP_MODE:
			handleSwitchSpMode(&dispMode);
			break;
		case CMD_RC5_DISPLAY:
			handleSetDefDisplay(&dispMode);
			break;
		case CMD_RC5_TIMER:
			handleChangeTimer(&dispMode, stbyTimer);
			break;
		case CMD_RC5_ALARM:
			handleEditAlarm(&dispMode);
			break;
		default:
			if (sndGetInput() == 0 && tunerGetType() != TUNER_NO) {
				switch (cmd) {
				case CMD_RC5_FM_CHAN_UP:
					handleChangeFM(&dispMode, SEARCH_UP);
					break;
				case CMD_RC5_FM_CHAN_DOWN:
					handleChangeFM(&dispMode, SEARCH_DOWN);
					break;
				case CMD_RC5_FM_TUNE:
					handleSwitchFmMode(&dispMode);
					break;
				case CMD_RC5_FM_MONO:
					tunerSwitchMono();
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				case CMD_RC5_FM_STORE:
					handleStoreStation(&dispMode);
					break;
				case CMD_RC5_FM_1:
				case CMD_RC5_FM_2:
				case CMD_RC5_FM_3:
				case CMD_RC5_FM_4:
				case CMD_RC5_FM_5:
				case CMD_RC5_FM_6:
				case CMD_RC5_FM_7:
				case CMD_RC5_FM_8:
				case CMD_RC5_FM_9:
				case CMD_RC5_FM_0:
					tunerLoadStation(cmd - CMD_RC5_FM_1);
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				}
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
			case MODE_TEMP:
				changeTempTH(encCnt);
				setDisplayTime(DISPLAY_TIME_TEMP);
				break;
			case MODE_TIME_EDIT:
				changeTime(encCnt);
				setDisplayTime(DISPLAY_TIME_TIME_EDIT);
				break;
			case MODE_ALARM_EDIT:
				changeAlarm(encCnt);
				setDisplayTime(DISPLAY_TIME_ALARM_EDIT);
				break;
			case MODE_BR:
				changeBrWork(encCnt);
				setDisplayTime(DISPLAY_TIME_BR);
				break;
			case MODE_FM_TUNE:
				tunerChangeFreq(encCnt);
				setDisplayTime(DISPLAY_TIME_FM_TUNE);
				break;
			case MODE_MUTE:
			case MODE_LOUDNESS:
			case MODE_SPECTRUM:
			case MODE_TIME:
			case MODE_TIMER:
			case MODE_FM_RADIO:
				dispMode = MODE_SND_VOLUME;
			default:
				sndSetMute(MUTE_OFF);
				sndChangeParam(dispMode, encCnt);
				setDisplayTime(DISPLAY_TIME_GAIN);
				break;
			}
		}

		/* Exit to default mode and save params to EEPROM*/
		if (getDisplayTime() == 0) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEMP:
				saveTempParams();
			case MODE_TEST:
				dispMode = MODE_STANDBY;
				break;
			default:
				if (getDefDisplay() == MODE_FM_RADIO) {
					if (sndGetInput() != 0 || tunerGetType() == TUNER_NO)
						dispMode = MODE_SPECTRUM;
					else
						dispMode = MODE_FM_RADIO;
				} else {
					dispMode = getDefDisplay();
				}
				break;
			}
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			gdClear();

		/* Switch to timer mode if it expires (less then minute) */
		if (stbyTimer >= 0 && stbyTimer <= 60 && getDisplayTime() == 0) {
			gdClear();
			dispMode = MODE_TIMER;
			setDisplayTime(DISPLAY_TIME_TIMER_EXP);
		}

		/* Show things */
		switch (dispMode) {
		case MODE_STANDBY:
			showTime(txtLabels);
			setStbyBrightness();
			break;
		case MODE_TEST:
			showRC5Info(txtLabels);
			setWorkBrightness();
			break;
		case MODE_TEMP:
			showTemp(txtLabels);
			setWorkBrightness();
			break;
		case MODE_SPECTRUM:
			drawSpectrum(getSpData(), txtLabels);
			break;
		case MODE_FM_RADIO:
			tunerReadStatus();
			showRadio(getSpData(), 0);
			break;
		case MODE_FM_TUNE:
			tunerReadStatus();
			showRadio(getSpData(), 1);
			break;
		case MODE_MUTE:
			showMute(txtLabels, getSpData());
			if (sndGetMute())
				setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case MODE_LOUDNESS:
			showLoudness(txtLabels, getSpData());
			break;
		case MODE_TIME:
		case MODE_TIME_EDIT:
			showTime(txtLabels);
			break;
		case MODE_TIMER:
			showTimer(getSpData());
			break;
		case MODE_ALARM:
		case MODE_ALARM_EDIT:
			showAlarm(txtLabels);
			break;
		case MODE_BR:
			showBrWork(txtLabels, getSpData());
			break;
		default:
			showSndParam(dispMode, txtLabels, getSpData());
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;
		/* Save current RC5 raw buf */
	}

	return 0;
}
