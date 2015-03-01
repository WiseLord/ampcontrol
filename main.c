#include <avr/interrupt.h>

#include "adc.h"
#include "input.h"
#include "rc5.h"
#include "i2c.h"

#include "display.h"
#include "tuner/tuner.h"
#include "temp.h"

#include "actions.h"

/* Hardware initialization */
static void hwInit(void)
{
	displayInit();						/* Load params and text labels before fb scan started */
	sei();								/* Gloabl interrupt enable */

	ds18x20SearchDevices();
	tempInit();							/* Init temperature control */

	inputInit();						/* Buttons/encoder polling */

	rc5Init();							/* IR Remote control */
	adcInit();							/* Analog-to-digital converter */
	I2CInit();							/* I2C bus */


	tunerInit();						/* Tuner */

	DDR(STMU_STBY) |= STMU_STBY_LINE;	/* Standby port */
	DDR(STMU_MUTE) |= STMU_MUTE_LINE;	/* Mute port */
	sndInit();					/* Load labels/icons/etc */

	powerOff();

	return;
}

int main(void)
{
	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = MODE_STANDBY;

	int8_t encCnt = 0;
	uint8_t cmd;
	uint8_t action = ACTION_NOACTION;

	int16_t stbyTimer = STBY_TIMER_OFF;

	hwInit();

	while (1) {
		encCnt = getEncoder();
		cmd = getBtnCmd();

		/* Emulate poweroff if standby timer expired */
		stbyTimer = getStbyTimer();
		if (stbyTimer == 0)
			action = ACTION_POWER_OFF;

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

		/* Remap input commands to internal commands */
		switch (cmd) {
		case CMD_BTN_1:
			if (dispMode == MODE_STANDBY)
				action = ACTION_POWER_ON;
			else
				action = ACTION_POWER_OFF;
			break;
		case CMD_BTN_2:
			action = ACTION_NEXT_INPUT;
			break;
		case CMD_BTN_3:
			if (dispMode == MODE_TIMER)
				action = ACTION_CHANGE_TIMER;
			else if (dispMode == MODE_FM_TUNE || dispMode == MODE_FM_RADIO)
				action = ACTION_DEC_FM;
			else if (dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT)
				action = ACTION_EDIT_TIME;
			else if (dispMode == MODE_ALARM || dispMode == MODE_ALARM_EDIT)
				action = ACTION_EDIT_ALARM;
			else
				action = ACTION_NEXT_SPMODE;
			break;
		case CMD_BTN_4:
			if (dispMode == MODE_FM_TUNE || dispMode == MODE_FM_RADIO)
				action = ACTION_INC_FM;
			else
				action = ACTION_SWITCH_MUTE;
			break;
		case CMD_BTN_5:
			if (dispMode == MODE_TEST)
				action = ACTION_NEXT_RC5_CMD;
			else {
				action = ACTION_NEXT_SNDPARAM;
			}
			break;

		case CMD_BTN_1_LONG:
			if (dispMode == MODE_TEST || dispMode == MODE_TEMP)
				action = ACTION_ZERO_DISPLAYTIME;
			else
				action = ACTION_BRIGHTNESS;
			break;
		case CMD_BTN_2_LONG:
			action = ACTION_DEF_DISPLAY;
			break;
		case CMD_BTN_3_LONG:
			if (dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT)
				action = ACTION_CHANGE_TIMER;
			else if (dispMode == MODE_TIMER)
				action = ACTION_EDIT_ALARM;
			else
				action = ACTION_EDIT_TIME;
			break;
		case CMD_BTN_4_LONG:
			action = ACTION_CHANGE_FM_MODE;
			break;
		case CMD_BTN_5_LONG:
			action = ACTION_STORE_FM_STATION;
			break;

		case CMD_BTN_12_LONG:
			action = ACTION_TESTMODE;
			break;
		case CMD_BTN_13_LONG:
			action = ACTION_TEMPMODE;
			break;

		case CMD_RC5_STBY:
			if (dispMode == MODE_STANDBY)
				action = ACTION_POWER_ON;
			else
				action = ACTION_POWER_OFF;
			break;
		case CMD_RC5_MUTE:
			action = ACTION_SWITCH_MUTE;
			break;
		case CMD_RC5_MENU:
			action = ACTION_NEXT_SNDPARAM;
			break;
		case CMD_RC5_VOL_UP:
			action = ACTION_VOLUME_UP;
			break;
		case CMD_RC5_VOL_DOWN:
			action = ACTION_VOLUME_DOWN;
			break;
		case CMD_RC5_INPUT_0:
			action = ACTION_INPUT_0;
			break;
		case CMD_RC5_INPUT_1:
			action = ACTION_INPUT_1;
			break;
		case CMD_RC5_INPUT_2:
			action = ACTION_INPUT_2;
			break;
		case CMD_RC5_INPUT_3:
			if (sndInputCnt() < 4) {
				action = ACTION_SWITCH_LOUDNESS;
			} else {
				action = ACTION_INPUT_3;
			}
			break;
		case CMD_RC5_NEXT_INPUT:
			action = ACTION_NEXT_INPUT;
			break;
		case CMD_RC5_TIME:
			action = ACTION_EDIT_TIME;
			break;
		case CMD_RC5_BACKLIGHT:
			action = ACTION_BRIGHTNESS;
			break;
		case CMD_RC5_SP_MODE:
			action = ACTION_NEXT_SPMODE;
			break;
		case CMD_RC5_FALLSPEED:
			action = ACTION_NEXT_FALLSPEED;
			break;
		case CMD_RC5_DISPLAY:
			action = ACTION_DEF_DISPLAY;
			break;
		case CMD_RC5_TIMER:
			action = ACTION_CHANGE_TIMER;
			break;
		case CMD_RC5_ALARM:
			action = ACTION_EDIT_ALARM;
			break;
		case CMD_RC5_FM_CHAN_UP:
			action = ACTION_INC_FM;
			break;
		case CMD_RC5_FM_CHAN_DOWN:
			action = ACTION_DEC_FM;
			break;
		case CMD_RC5_FM_TUNE:
			action = ACTION_CHANGE_FM_MODE;
			break;
		case CMD_RC5_FM_MONO:
			action = ACTION_SWITCH_FM_MONO;
			break;
		case CMD_RC5_FM_STORE:
			action = ACTION_STORE_FM_STATION;
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
			action = ACTION_FM_STATION_0 + (cmd - CMD_RC5_FM_0);
			break;
		}

		/* Don't handle any command in test mode except CMD_NEXT_RC5_CMD and CMD_ZERO_DISPLAY_TIME */
		if (dispMode == MODE_TEST) {
			if (action != ACTION_NOACTION)
				setDisplayTime(DISPLAY_TIME_TEST);
			if (action != ACTION_NEXT_RC5_CMD && action != ACTION_ZERO_DISPLAYTIME)
				action = ACTION_NOACTION;
		}
		/* Don't handle any command in temp mode except CMD_ZERO_DISPLAY_TIME */
		if (dispMode == MODE_TEMP) {
			if (action != ACTION_NOACTION)
				setDisplayTime(DISPLAY_TIME_TEMP);
			if (action != ACTION_ZERO_DISPLAYTIME)
				action = ACTION_NOACTION;
		}
		/* Don't handle commands in standby mode except POWERON, TESTMODE and TEMPMODE */
		if (dispMode == MODE_STANDBY) {
			if (action != ACTION_POWER_ON && action != ACTION_TESTMODE && action != ACTION_TEMPMODE)
				action = ACTION_NOACTION;
		}

		/* Handle command */
		switch (action) {
		case ACTION_POWER_ON:
			powerOn();
			dispMode = getDefDisplay();
			break;
		case ACTION_POWER_OFF:
			powerOff();
			dispMode = MODE_STANDBY;
			break;
		case ACTION_NEXT_INPUT:
			if (dispMode >= MODE_SND_GAIN0 && dispMode <= MODE_SND_GAIN3)
				sndSetInput(sndGetInput() + 1);
			handleSetInput(&dispMode);
			break;
		case ACTION_CHANGE_TIMER:
			stopEditTime();
			if (dispMode == MODE_TIMER) {
				setSecTimer(2000);
				if (stbyTimer < 120)		/* 2 min */
					setStbyTimer(120);
				else if (stbyTimer < 300)	/* 5 min */
					setStbyTimer(300);
				else if (stbyTimer < 600)	/* 10 min */
					setStbyTimer(600);
				else if (stbyTimer < 1200)	/* 20 min */
					setStbyTimer(1200);
				else if (stbyTimer < 2400)	/* 40 min */
					setStbyTimer(2400);
				else if (stbyTimer < 3600)	/* 1 hour */
					setStbyTimer(3600);
				else if (stbyTimer < 5400)	/* 1.5 hours */
					setStbyTimer(5400);
				else if (stbyTimer < 7200)	/* 2 hours */
					setStbyTimer(7200);
				else if (stbyTimer < 10800)	/* 3 hours */
					setStbyTimer(10800);
				else if (stbyTimer < 18000)	/* 5 hours */
					setStbyTimer(18000);
				else
					setStbyTimer(STBY_TIMER_OFF);
			}
			dispMode = MODE_TIMER;
			setDisplayTime(DISPLAY_TIME_TIMER);
			break;
		case ACTION_EDIT_TIME:
			handleEditTime(&dispMode);
			break;
		case ACTION_EDIT_ALARM:
			handleEditAlarm(&dispMode);
			break;
		case ACTION_NEXT_SPMODE:
			switchSpMode();
			gdClear();
			dispMode = MODE_SPECTRUM;
			setDisplayTime(DISPLAY_TIME_SP);
			break;
		case ACTION_SWITCH_MUTE:
			handleSwitchMute(&dispMode);
			break;
		case ACTION_NEXT_RC5_CMD:
			gdClear();
			nextRC5Cmd();
			break;
		case ACTION_NEXT_SNDPARAM:
			sndNextParam(&dispMode);
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case ACTION_ZERO_DISPLAYTIME:
			setDisplayTime(0);
			break;
		case ACTION_BRIGHTNESS:
			dispMode = MODE_BR;
			setDisplayTime(DISPLAY_TIME_BR);
			break;
		case ACTION_DEF_DISPLAY:
			handleSetDefDisplay(&dispMode);
			break;
		case ACTION_INPUT_0:
		case ACTION_INPUT_1:
		case ACTION_INPUT_2:
		case ACTION_INPUT_3:
			sndSetInput(action - ACTION_INPUT_0);
			handleSetInput(&dispMode);
			break;
		case ACTION_SWITCH_LOUDNESS:
			sndSetLoudness(!sndGetLoudness());
			dispMode = MODE_LOUDNESS;
			setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case ACTION_NEXT_FALLSPEED:
			switchFallSpeed();
			dispMode = MODE_SPECTRUM;
			setDisplayTime(DISPLAY_TIME_SP);
			break;
		case ACTION_TESTMODE:
			switch (dispMode) {
			case MODE_STANDBY:
				dispMode = MODE_TEST;
				startTestMode();
				setDisplayTime(DISPLAY_TIME_TEST);
				break;
			}
			break;
		case ACTION_TEMPMODE:
			switch (dispMode) {
			case MODE_STANDBY:
				dispMode = MODE_TEMP;
				setDisplayTime(DISPLAY_TIME_TEMP);
				break;
			}
			break;
		default:
			if (sndGetInput() == 0 && tunerGetType() != TUNER_NO) {
				switch (action) {
				case ACTION_FM_STATION_1:
				case ACTION_FM_STATION_2:
				case ACTION_FM_STATION_3:
				case ACTION_FM_STATION_4:
				case ACTION_FM_STATION_5:
				case ACTION_FM_STATION_6:
				case ACTION_FM_STATION_7:
				case ACTION_FM_STATION_8:
				case ACTION_FM_STATION_9:
				case ACTION_FM_STATION_0:
					tunerLoadStation(action - ACTION_FM_STATION_1);
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				case ACTION_INC_FM:
					handleChangeFM(&dispMode, SEARCH_UP);
					break;
				case ACTION_DEC_FM:
					handleChangeFM(&dispMode, SEARCH_DOWN);
					break;
				case ACTION_CHANGE_FM_MODE:
					handleSwitchFmMode(&dispMode);
					break;
				case ACTION_STORE_FM_STATION:
					handleStoreStation(&dispMode);
					break;
				case ACTION_SWITCH_FM_MONO:
					tunerSwitchMono();
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				}
			}
			break;
		}

		/* Emulate RC5 VOL_UP/VOL_DOWN as encoder actions */
		if (action == ACTION_VOLUME_UP)
			encCnt++;
		if (action == ACTION_VOLUME_DOWN)
			encCnt--;

		action = ACTION_NOACTION;

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
			showTime();
			setStbyBrightness();
			break;
		case MODE_TEST:
			showRC5Info();
			setWorkBrightness();
			break;
		case MODE_TEMP:
			showTemp();
			setWorkBrightness();
			break;
		case MODE_SPECTRUM:
			showSpectrum();
			break;
		case MODE_FM_RADIO:
			showRadio(MODE_RADIO_CHAN);
			break;
		case MODE_FM_TUNE:
			showRadio(MODE_RADIO_TUNE);
			break;
		case MODE_MUTE:
			showMute();
			if (sndGetMute())
				setDisplayTime(DISPLAY_TIME_AUDIO);
			break;
		case MODE_LOUDNESS:
			showLoudness();
			break;
		case MODE_TIME:
		case MODE_TIME_EDIT:
			showTime();
			break;
		case MODE_TIMER:
			showTimer();
			break;
		case MODE_ALARM:
		case MODE_ALARM_EDIT:
			showAlarm();
			break;
		case MODE_BR:
			showBrWork();
			break;
		default:
			showSndParam(dispMode);
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;
		/* Save current RC5 raw buf */
	}

	return 0;
}
