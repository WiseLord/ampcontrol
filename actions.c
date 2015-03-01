#include "actions.h"

#include <util/delay.h>
#include "display.h"
#include "tuner/tuner.h"
#include "temp.h"

actionID getAction(uint8_t *dispMode)
{
	actionID action = ACTION_NOACTION;
	cmdID cmd;

	/* Get command */
	cmd = getBtnCmd();

	/* Handle commands from remote control */
	if (cmd < CMD_RC5_END)
		action = cmd;

	/* Handle commands from buttons*/
	switch (cmd) {
	case CMD_BTN_1:
		action = ACTION_GO_STANDBY;
		break;
	case CMD_BTN_2:
		action = ACTION_NEXT_INPUT;
		break;
	case CMD_BTN_3:
		if (*dispMode == MODE_TIMER)
			action = ACTION_CHANGE_TIMER;
		else if (*dispMode == MODE_FM_TUNE || *dispMode == MODE_FM_RADIO)
			action = ACTION_DEC_FM;
		else if (*dispMode == MODE_TIME || *dispMode == MODE_TIME_EDIT)
			action = ACTION_EDIT_TIME;
		else if (*dispMode == MODE_ALARM || *dispMode == MODE_ALARM_EDIT)
			action = ACTION_EDIT_ALARM;
		else
			action = ACTION_NEXT_SPMODE;
		break;
	case CMD_BTN_4:
		if (*dispMode == MODE_FM_TUNE || *dispMode == MODE_FM_RADIO)
			action = ACTION_INC_FM;
		else
			action = ACTION_SWITCH_MUTE;
		break;
	case CMD_BTN_5:
		if (*dispMode == MODE_TEST)
			action = ACTION_NEXT_RC5_CMD;
		else {
			action = ACTION_NEXT_SNDPARAM;
		}
		break;

	case CMD_BTN_1_LONG:
		if (*dispMode == MODE_TEST || *dispMode == MODE_TEMP)
			action = ACTION_ZERO_DISPLAYTIME;
		else
			action = ACTION_BRIGHTNESS;
		break;
	case CMD_BTN_2_LONG:
		action = ACTION_DEF_DISPLAY;
		break;
	case CMD_BTN_3_LONG:
		if (*dispMode == MODE_TIME || *dispMode == MODE_TIME_EDIT)
			action = ACTION_CHANGE_TIMER;
		else if (*dispMode == MODE_TIMER)
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
	default:
		break;
	}

	/* Remap GO_STANDBY command to EXIT_STANDBY if in standby mode */
	if (action == ACTION_GO_STANDBY && *dispMode == MODE_STANDBY)
		action = ACTION_EXIT_STANDBY;

	/* Remap INPUT_3 command to SWITCH_LOUDNESS if there is no INPUT_3 */
	if (action == ACTION_INPUT_3 && sndInputCnt() < 4)
		action = ACTION_SWITCH_LOUDNESS;

	/* Remap NEXT_INPUT action to INPUT_X */
	if (action == ACTION_NEXT_INPUT) {
		action = ACTION_INPUT_0 + sndGetInput();
		if (*dispMode >= MODE_SND_GAIN0 && *dispMode <= MODE_SND_GAIN3) {
			action += 1;
			if (action > ACTION_INPUT_3)
				action = ACTION_INPUT_0;
		}
	}

	/* Disable actions except NEXT_RC5_CMD and ZERO_DISPLAY_TIME in test mode */
	if (*dispMode == MODE_TEST) {
		if (action != ACTION_NOACTION)
			setDisplayTime(DISPLAY_TIME_TEST);
		if (action != ACTION_NEXT_RC5_CMD && action != ACTION_ZERO_DISPLAYTIME)
			action = ACTION_NOACTION;
	}
	/* Disable actions except ZERO_DISPLAY_TIME in temp mode */
	if (*dispMode == MODE_TEMP) {
		if (action != ACTION_NOACTION)
			setDisplayTime(DISPLAY_TIME_TEMP);
		if (action != ACTION_ZERO_DISPLAYTIME)
			action = ACTION_NOACTION;
	}
	/* Disable actions except POWERON, TESTMODE and TEMPMODE in standby mode */
	if (*dispMode == MODE_STANDBY) {
		if (action != ACTION_EXIT_STANDBY && action != ACTION_TESTMODE && action != ACTION_TEMPMODE)
			action = ACTION_NOACTION;
	}

	return action;
}


void handleAction(actionID action, uint8_t *dispMode)
{
	int16_t stbyTimer = STBY_TIMER_OFF;

	switch (action) {
	case ACTION_EXIT_STANDBY:
		PORT(STMU_STBY) |= STMU_STBY_LINE;	/* Power up audio and tuner */
		setWorkBrightness();

		_delay_ms(100);						/* Wait while power is being set up */

		tunerPowerOn();
		sndPowerOn();

		if (sndGetInput() == 0)
			tunerSetMute(MUTE_OFF);
		else
			tunerSetMute(MUTE_ON);

		*dispMode = getDefDisplay();
		break;
	case ACTION_GO_STANDBY:
		sndSetMute(MUTE_ON);

		_delay_ms(100);

		PORT(STMU_STBY) &= ~STMU_STBY_LINE;

		setStbyBrightness();
		stopEditTime();
		setStbyTimer(STBY_TIMER_OFF);

		sndPowerOff();
		tunerPowerOff();
		displayPowerOff();
		*dispMode = MODE_STANDBY;
		break;
	case ACTION_CHANGE_TIMER:
		stopEditTime();
		if (*dispMode == MODE_TIMER) {
			setSecTimer(2000);
			stbyTimer = getStbyTimer();
			if (stbyTimer < 120)			/* 2 min */
				setStbyTimer(120);
			else if (stbyTimer < 300)		/* 5 min */
				setStbyTimer(300);
			else if (stbyTimer < 600)		/* 10 min */
				setStbyTimer(600);
			else if (stbyTimer < 1200)		/* 20 min */
				setStbyTimer(1200);
			else if (stbyTimer < 2400)		/* 40 min */
				setStbyTimer(2400);
			else if (stbyTimer < 3600)		/* 1 hour */
				setStbyTimer(3600);
			else if (stbyTimer < 5400)		/* 1.5 hours */
				setStbyTimer(5400);
			else if (stbyTimer < 7200)		/* 2 hours */
				setStbyTimer(7200);
			else if (stbyTimer < 10800)		/* 3 hours */
				setStbyTimer(10800);
			else if (stbyTimer < 18000)		/* 5 hours */
				setStbyTimer(18000);
			else
				setStbyTimer(STBY_TIMER_OFF);
		}
		*dispMode = MODE_TIMER;
		setDisplayTime(DISPLAY_TIME_TIMER);
		break;
	case ACTION_EDIT_TIME:
		if (*dispMode == MODE_TIME || *dispMode == MODE_TIME_EDIT) {
			editTime();
			*dispMode = MODE_TIME_EDIT;
			setDisplayTime(DISPLAY_TIME_TIME_EDIT);
			if (!isETM())
				setDisplayTime(DISPLAY_TIME_TIME);
		} else {
			stopEditTime();
			*dispMode = MODE_TIME;
			setDisplayTime(DISPLAY_TIME_TIME);
		}
		break;
	case ACTION_EDIT_ALARM:
		if (*dispMode == MODE_ALARM || *dispMode == MODE_ALARM_EDIT) {
			editAlarm();
			*dispMode = MODE_ALARM_EDIT;
			setDisplayTime(DISPLAY_TIME_ALARM_EDIT);
			if (!isEAM())
				setDisplayTime(DISPLAY_TIME_ALARM);
		} else {
			stopEditAlarm();
			*dispMode = MODE_ALARM;
			setDisplayTime(DISPLAY_TIME_ALARM);
		}
		break;
	case ACTION_NEXT_SPMODE:
		switchSpMode();
		gdClear();
		*dispMode = MODE_SPECTRUM;
		setDisplayTime(DISPLAY_TIME_SP);
		break;
	case ACTION_SWITCH_MUTE:
		gdClear();
		sndSetMute(!sndGetMute());
		*dispMode = MODE_MUTE;
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case ACTION_NEXT_RC5_CMD:
		gdClear();
		nextRC5Cmd();
		break;
	case ACTION_NEXT_SNDPARAM:
		sndNextParam(dispMode);
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case ACTION_ZERO_DISPLAYTIME:
		setDisplayTime(0);
		break;
	case ACTION_BRIGHTNESS:
		*dispMode = MODE_BR;
		setDisplayTime(DISPLAY_TIME_BR);
		break;
	case ACTION_DEF_DISPLAY:
		switch (getDefDisplay()) {
		case MODE_SPECTRUM:
			setDefDisplay(MODE_TIME);
			break;
		case MODE_TIME:
			if (sndGetInput() == 0 && tunerGetType() != TUNER_NO) {
				setDefDisplay(MODE_FM_RADIO);
				break;
			}
		default:
			setDefDisplay(MODE_SPECTRUM);
			break;
		}
		*dispMode = getDefDisplay();
		break;
	case ACTION_INPUT_0:
	case ACTION_INPUT_1:
	case ACTION_INPUT_2:
	case ACTION_INPUT_3:
		sndSetInput(action - ACTION_INPUT_0);
		*dispMode = MODE_SND_GAIN0 + sndGetInput();
		setDisplayTime(DISPLAY_TIME_GAIN);
		if (sndGetInput() == 0)
			tunerSetMute(MUTE_OFF);
		else
			tunerSetMute(MUTE_ON);
		break;
	case ACTION_SWITCH_LOUDNESS:
		sndSetLoudness(!sndGetLoudness());
		*dispMode = MODE_LOUDNESS;
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case ACTION_NEXT_FALLSPEED:
		switchFallSpeed();
		*dispMode = MODE_SPECTRUM;
		setDisplayTime(DISPLAY_TIME_SP);
		break;
	case ACTION_TESTMODE:
		switch (*dispMode) {
		case MODE_STANDBY:
			*dispMode = MODE_TEST;
			startTestMode();
			setDisplayTime(DISPLAY_TIME_TEST);
			break;
		}
		break;
	case ACTION_TEMPMODE:
		switch (*dispMode) {
		case MODE_STANDBY:
			*dispMode = MODE_TEMP;
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
				*dispMode = MODE_FM_RADIO;
				setDisplayTime(DISPLAY_TIME_FM_RADIO);
				break;
			case ACTION_INC_FM:
				handleChangeFM(dispMode, SEARCH_UP);
				break;
			case ACTION_DEC_FM:
				handleChangeFM(dispMode, SEARCH_DOWN);
				break;
			case ACTION_CHANGE_FM_MODE:
				switch (*dispMode) {
				case MODE_FM_RADIO:
					*dispMode = MODE_FM_TUNE;
					setDisplayTime(DISPLAY_TIME_FM_TUNE);
					break;
				case MODE_FM_TUNE:
					*dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
				}
				break;
			case ACTION_STORE_FM_STATION:
				if (*dispMode == MODE_FM_TUNE) {
					setDisplayTime(DISPLAY_TIME_FM_TUNE);
					tunerStoreStation();
				}
				break;
			case ACTION_SWITCH_FM_MONO:
				tunerSwitchMono();
				*dispMode = MODE_FM_RADIO;
				setDisplayTime(DISPLAY_TIME_FM_RADIO);
				break;
			default:
				break;
			}
		}
		break;
	}

}

void handleEncoder(int8_t encCnt, uint8_t *dispMode)
{
	if (encCnt) {
		switch (*dispMode) {
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
			*dispMode = MODE_SND_VOLUME;
		default:
			sndSetMute(MUTE_OFF);
			sndChangeParam(*dispMode, encCnt);
			setDisplayTime(DISPLAY_TIME_GAIN);
			break;
		}
	}

	return;
}

void handleChangeFM(uint8_t *dispMode, uint8_t step)
{
	if (*dispMode == MODE_FM_TUNE) {
		tunerChangeFreq(step * 10);
		setDisplayTime(DISPLAY_TIME_FM_TUNE);
	} else {
		tunerNextStation(step);
		*dispMode = MODE_FM_RADIO;
		setDisplayTime(DISPLAY_TIME_FM_RADIO);
	}

	return;
}

actionID checkAlarmAndTime(uint8_t *dispMode)
{
	actionID ret = ACTION_NOACTION;

	if (getClockTimer() == 0) {
		readTime();
		readAlarm();

		if (*dispMode == MODE_STANDBY) {
			if ((getTime(DS1307_SEC) == 0) &&
			    (getTime(DS1307_MIN) == getAlarm(DS1307_A0_MIN)) &&
			    (getTime(DS1307_HOUR) == getAlarm(DS1307_A0_HOUR)) &&
			    (getAlarm(DS1307_A0_WDAY) & (0x40 >> ((getTime(DS1307_WDAY) + 5) % 7)))
			   ) {
				sndSetInput(getAlarm(DS1307_A0_INPUT));
				ret = ACTION_EXIT_STANDBY;
			}
		}

		setClockTimer(200);					/* Limit check interval */
	}

	return ret;
}

void handleExitDefaultMode(uint8_t *dispMode)
{
	if (getDisplayTime() == 0) {
		switch (*dispMode) {
		case MODE_STANDBY:
			break;
		case MODE_TEMP:
			saveTempParams();
		case MODE_TEST:
			*dispMode = MODE_STANDBY;
			break;
		default:
			if (getDefDisplay() == MODE_FM_RADIO) {
				if (sndGetInput() != 0 || tunerGetType() == TUNER_NO)
					*dispMode = MODE_SPECTRUM;
				else
					*dispMode = MODE_FM_RADIO;
			} else {
				*dispMode = getDefDisplay();
			}
			break;
		}
	}

	return;
}

void handleTimerExpires(uint8_t *dispMode)
{
	if (getStbyTimer() >= 0 && getStbyTimer() <= 60 && getDisplayTime() == 0) {
		*dispMode = MODE_TIMER;
		setDisplayTime(DISPLAY_TIME_TIMER_EXP);
	}

	return;
}

void handleModeChange(uint8_t *dispMode, uint8_t *dispModePrev)
{
	if (*dispMode != *dispModePrev)
		gdClear();

	return;
}

void showScreen(uint8_t *dispMode, uint8_t *dispModePrev)
{
	switch (*dispMode) {
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
		showSndParam(*dispMode);
		break;
	}

	/* Save current mode */
	*dispModePrev = *dispMode;

	return;
}
