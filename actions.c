#include "actions.h"

#include <util/delay.h>
#include "display.h"
#include "tuner/tuner.h"
#include "temp.h"
#include "adc.h"
#include "alarm.h"
#include "uart.h"

static uint8_t dispMode = MODE_STANDBY;
static uint8_t dispModePrev = MODE_STANDBY;

static uint8_t defDispMode(void)
{
	uint8_t ret;

	if (getDefDisplay() == MODE_FM_RADIO) {
		if (aproc.input || !tuner.ic)
			ret = MODE_SPECTRUM;
		else
			ret = MODE_FM_RADIO;
	} else {
		ret = getDefDisplay();
	}
	if (aproc.mute)
		ret = MODE_MUTE;

	return ret;
}

uint8_t getAction(void)
{
	uint8_t action = ACTION_NOACTION;
	cmdID cmd;

	/* Get command */
	cmd = getBtnCmd();

	// If nothing from buttons, check remote control
	if (cmd == CMD_RC_END)
		cmd = getRcCmd();

	/* Handle commands from remote control */
	if (cmd < CMD_RC_END)
		action = cmd;

	/* Handle commands from UART */
	UARTData uartData = getUartData();
	if (uartData.type == UART_CMD_RC) {
		if (uartData.command < CMD_RC_END)
			action = uartData.command;
	}

	/* Handle commands from buttons*/
	switch (cmd) {
	case CMD_BTN_1:
		action = CMD_RC_STBY;
		break;
	case CMD_BTN_2:
		action = CMD_RC_IN_NEXT;
		break;
	case CMD_BTN_3:
		if (dispMode == MODE_TIMER)
			action = CMD_RC_TIMER;
		else if (dispMode == MODE_FM_TUNE || dispMode == MODE_FM_RADIO)
			action = CMD_RC_FM_DEC;
		else if (dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT)
			action = CMD_RC_TIME;
		else if (dispMode == MODE_ALARM || dispMode == MODE_ALARM_EDIT)
			action = CMD_RC_ALARM;
		else
			action = CMD_RC_NEXT_SPMODE;
		break;
	case CMD_BTN_4:
		if (dispMode == MODE_FM_TUNE || dispMode == MODE_FM_RADIO)
			action = CMD_RC_FM_INC;
		else
			action = CMD_RC_MUTE;
		break;
	case CMD_BTN_5:
		if (dispMode == MODE_TEST) {
			action = ACTION_NEXT_RC_CMD;
		} else if (dispMode == MODE_FM_TUNE) {
			action = CMD_RC_FM_STORE;
		} else {
			action = CMD_RC_NEXT_SNDPAR;
		}
		break;

	case CMD_BTN_1_LONG:
		if (dispMode == MODE_TEST || dispMode == MODE_TEMP)
			action = ACTION_ZERO_DISPLAYTIME;
		else
			action = CMD_RC_BRIGHTNESS;
		break;
	case CMD_BTN_2_LONG:
		action = CMD_RC_DEF_DISPLAY;
		break;
	case CMD_BTN_3_LONG:
		if (dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT)
			action = CMD_RC_TIMER;
		else if (dispMode == MODE_TIMER)
			action = CMD_RC_ALARM;
		else
			action = CMD_RC_TIME;
		break;
	case CMD_BTN_4_LONG:
		action = CMD_RC_FM_MODE;
		break;
	case CMD_BTN_5_LONG:
		// Reserved for future purposes
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
	if (action == CMD_RC_STBY && dispMode == MODE_STANDBY)
		action = ACTION_EXIT_STANDBY;

	/* Remap NEXT/PREV_INPUT actions to INPUT_X */
	if (action == CMD_RC_IN_NEXT) {
		action = CMD_RC_IN_0 + aproc.input;
		if (dispMode >= MODE_SND_GAIN0 && dispMode < MODE_SND_END) {
			action += 1;
			if (action >= CMD_RC_IN_0 + aproc.inCnt)
				action = CMD_RC_IN_0;
		}
	}
	if (action == CMD_RC_IN_PREV) {
		action = CMD_RC_IN_0 + aproc.input;
		if (dispMode >= MODE_SND_GAIN0 && dispMode < MODE_SND_END) {
			action -= 1;
			if (action < CMD_RC_IN_0)
				action += aproc.inCnt;
		}
	}

	/* Disable actions except NEXT_RC_CMD and ZERO_DISPLAY_TIME in test mode */
	if (dispMode == MODE_TEST) {
		if (action != ACTION_NOACTION)
			setDisplayTime(DISPLAY_TIME_TEST);
		if (action != ACTION_NEXT_RC_CMD && action != ACTION_ZERO_DISPLAYTIME)
			action = ACTION_NOACTION;
	}
	/* Disable actions except ZERO_DISPLAY_TIME in temp mode */
	if (dispMode == MODE_TEMP) {
		if (action != ACTION_NOACTION)
			setDisplayTime(DISPLAY_TIME_TEMP);
		if (action != ACTION_ZERO_DISPLAYTIME)
			action = ACTION_NOACTION;
	}
	/* Disable actions except POWERON, TESTMODE and TEMPMODE in standby mode */
	if (dispMode == MODE_STANDBY) {
		if (action != ACTION_EXIT_STANDBY && action != ACTION_TESTMODE && action != ACTION_TEMPMODE)
			action = ACTION_NOACTION;
	}
	/* Disable most action in time edit mode */
	if (dispMode == MODE_TIME_EDIT) {
		if (action != CMD_RC_STBY && action != CMD_RC_TIME &&
			action != CMD_RC_VOL_DOWN && action != CMD_RC_VOL_UP)
			action = ACTION_NOACTION;
	}
	/* Disable most actions in alarm edit mode */
	if (dispMode == MODE_ALARM_EDIT) {
		if (action != CMD_RC_STBY && action != CMD_RC_ALARM &&
			action != CMD_RC_VOL_DOWN && action != CMD_RC_VOL_UP)
			action = ACTION_NOACTION;
	}
	/* Disable most actions in FM edit mode */
	if (dispMode == MODE_FM_TUNE) {
		if (action != CMD_RC_STBY &&
			action != CMD_RC_VOL_DOWN && action != CMD_RC_VOL_UP &&
			action != CMD_RC_FM_MODE && action != CMD_RC_FM_STORE &&
			action != CMD_RC_FM_DEC && action != CMD_RC_FM_INC &&
			(action < CMD_RC_FM_0 || action > CMD_RC_FM_9)
			)
			action = ACTION_NOACTION;
	}

	return action;
}


void handleAction(uint8_t action)
{
	int16_t stbyTimer = STBY_TIMER_OFF;

	switch (action) {
	case ACTION_EXIT_STANDBY:
		PORT(STMU_STBY) |= STMU_STBY_LINE;	/* Power up audio and tuner */
		setWorkBrightness();

		setInitTimer(INIT_TIMER_START);

		dispMode = MODE_SND_GAIN0 + aproc.input;
		setDisplayTime(DISPLAY_TIME_GAIN_START);
		enableSilenceTimer();

		break;
	case ACTION_INIT_HARDWARE:
		tunerPowerOn();
		sndPowerOn();

		tunerSetMute(aproc.input);
		tunerSetFreq();

		setInitTimer(INIT_TIMER_OFF);
		break;
	case CMD_RC_STBY:
		sndSetMute(1);
		sndPowerOff();
		tunerPowerOff();
		displayPowerOff();

		PORT(STMU_STBY) &= ~STMU_STBY_LINE;

		setStbyBrightness();
		rtc.etm = RTC_NOEDIT;
		alarmSave();
		setStbyTimer(STBY_TIMER_OFF);
		disableSilenceTimer();
		setInitTimer(INIT_TIMER_OFF);
		dispMode = MODE_STANDBY;
		break;
	case CMD_RC_TIMER:
		rtc.etm = RTC_NOEDIT;
		if (dispMode == MODE_TIMER) {
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
		dispMode = MODE_TIMER;
		setDisplayTime(DISPLAY_TIME_TIMER);
		break;
	case CMD_RC_TIME:
		if ((dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT) && rtc.etm != RTC_YEAR) {
			rtcNextEditParam();
			dispMode = MODE_TIME_EDIT;
			setDisplayTime(DISPLAY_TIME_TIME_EDIT);
		} else {
			rtc.etm = RTC_NOEDIT;
			dispMode = MODE_TIME;
			setDisplayTime(DISPLAY_TIME_TIME);
		}
		break;
	case CMD_RC_ALARM:
		if ((dispMode == MODE_ALARM || dispMode == MODE_ALARM_EDIT) && alarm0.eam != ALARM_WDAY) {
			alarmNextEditParam();
			dispMode = MODE_ALARM_EDIT;
			setDisplayTime(DISPLAY_TIME_ALARM_EDIT);
		} else {
			alarmSave();
			dispMode = MODE_ALARM;
			setDisplayTime(DISPLAY_TIME_ALARM);
		}
		break;
	case CMD_RC_NEXT_SPMODE:
		switchSpMode();
		displayClear();
		dispMode = MODE_SPECTRUM;
		setDisplayTime(DISPLAY_TIME_SP);
		break;
	case CMD_RC_MUTE:
		dispMode = MODE_MUTE;
		if (aproc.mute) {
			sndSetMute(0);
			tunerSetMute(aproc.input);
			setDisplayTime(DISPLAY_TIME_AUDIO);
		} else {
			tunerSetMute(1);
			sndSetMute(1);
		}
		break;
	case ACTION_NEXT_RC_CMD:
		displayClear();
		nextRcCmd();
		break;
	case CMD_RC_NEXT_SNDPAR:
		sndNextParam(&dispMode);
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case ACTION_ZERO_DISPLAYTIME:
		setDisplayTime(0);
		break;
	case CMD_RC_BRIGHTNESS:
		dispMode = MODE_BR;
		setDisplayTime(DISPLAY_TIME_BR);
		break;
	case CMD_RC_DEF_DISPLAY:
		switch (getDefDisplay()) {
		case MODE_TIME:
			setDefDisplay(MODE_SPECTRUM);
			break;
		case MODE_SPECTRUM:
			if (!aproc.input && tuner.ic) {
				setDefDisplay(MODE_FM_RADIO);
				break;
			}
		default:
			setDefDisplay(MODE_TIME);
			break;
		}
		dispMode = getDefDisplay();
		setDisplayTime(DISPLAY_TIME_SP);
		break;
	case CMD_RC_IN_0:
		if (getDefDisplay() == MODE_SPECTRUM && tuner.ic)
			setDefDisplay(MODE_FM_RADIO);
	case CMD_RC_IN_1:
	case CMD_RC_IN_2:
	case CMD_RC_IN_3:
	case CMD_RC_IN_4:
		sndSetInput(action - CMD_RC_IN_0);
		dispMode = MODE_SND_GAIN0 + aproc.input;
		setDisplayTime(DISPLAY_TIME_GAIN);
		tunerSetMute(aproc.mute || aproc.input);
		break;
	case CMD_RC_LOUDNESS:
		sndSwitchExtra(APROC_EXTRA_LOUDNESS);
		dispMode = MODE_LOUDNESS;
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case CMD_RC_SURROUND:
		sndSwitchExtra(APROC_EXTRA_SURROUND);
		dispMode = MODE_SURROUND;
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case CMD_RC_EFFECT_3D:
		sndSwitchExtra(APROC_EXTRA_EFFECT3D);
		dispMode = MODE_EFFECT_3D;
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case CMD_RC_TONE_DEFEAT:
		sndSwitchExtra(APROC_EXTRA_TONEDEFEAT);
		dispMode = MODE_TONE_DEFEAT;
		setDisplayTime(DISPLAY_TIME_AUDIO);
		break;
	case CMD_RC_FALLSPEED:
		switchFallSpeed();
		dispMode = MODE_SPECTRUM;
		setDisplayTime(DISPLAY_TIME_SP);
		break;
	case ACTION_TESTMODE:
		switch (dispMode) {
		case MODE_STANDBY:
			dispMode = MODE_TEST;
			setWorkBrightness();
			switchTestMode(CMD_RC_STBY);
			setDisplayTime(DISPLAY_TIME_TEST);
			break;
		}
		break;
	case ACTION_TEMPMODE:
		switch (dispMode) {
		case MODE_STANDBY:
			dispMode = MODE_TEMP;
			setWorkBrightness();
			setDisplayTime(DISPLAY_TIME_TEMP);
			break;
		}
		break;
	default:
		if (!aproc.input && tuner.ic) {
			switch (action) {
			case CMD_RC_FM_0:
			case CMD_RC_FM_1:
			case CMD_RC_FM_2:
			case CMD_RC_FM_3:
			case CMD_RC_FM_4:
			case CMD_RC_FM_5:
			case CMD_RC_FM_6:
			case CMD_RC_FM_7:
			case CMD_RC_FM_8:
			case CMD_RC_FM_9:
				if (dispMode == MODE_FM_TUNE) {
					tunerStoreFavStation(action - CMD_RC_FM_0);
					setDisplayTime(DISPLAY_TIME_FM_TUNE);
				} else {
					tunerLoadFavStation(action - CMD_RC_FM_0);
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
				}
				break;
			case CMD_RC_FM_INC:
				handleChangeFM(SEARCH_UP);
				break;
			case CMD_RC_FM_DEC:
				handleChangeFM(SEARCH_DOWN);
				break;
			case CMD_RC_FM_MODE:
				switch (dispMode) {
				case MODE_FM_RADIO:
					dispMode = MODE_FM_TUNE;
					setDisplayTime(DISPLAY_TIME_FM_TUNE);
					break;
				case MODE_FM_TUNE:
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
				}
				break;
			case CMD_RC_FM_STORE:
				if (dispMode == MODE_FM_TUNE) {
					setDisplayTime(DISPLAY_TIME_FM_TUNE);
					tunerStoreStation();
				}
				break;
			case CMD_RC_FM_MONO:
				tunerSetMono(!tuner.mono);
				dispMode = MODE_FM_RADIO;
				setDisplayTime(DISPLAY_TIME_FM_RADIO);
				break;
			case CMD_RC_FM_RDS:
				tunerSetRDS(!tuner.rds);
				dispMode = MODE_FM_RADIO;
				setDisplayTime(DISPLAY_TIME_FM_RADIO);
			default:
				break;
			}
		}
		break;
	}

}

void handleEncoder(int8_t encCnt)
{
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
			rtcChangeTime(encCnt);
			setDisplayTime(DISPLAY_TIME_TIME_EDIT);
			break;
		case MODE_ALARM_EDIT:
			alarmChangeTime(encCnt);
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
		case MODE_SURROUND:
		case MODE_EFFECT_3D:
		case MODE_TONE_DEFEAT:
		case MODE_SPECTRUM:
		case MODE_TIME:
		case MODE_TIMER:
		case MODE_SILENCE_TIMER:
		case MODE_ALARM:
		case MODE_FM_RADIO:
			dispMode = MODE_SND_VOLUME;
		default:
			sndSetMute(0);
			sndChangeParam(dispMode, encCnt);
			setDisplayTime(DISPLAY_TIME_GAIN);
			break;
		}
	}

	return;
}

void handleChangeFM(uint8_t step)
{
	if (dispMode == MODE_FM_TUNE) {
		tunerChangeFreq(step * 10);
		setDisplayTime(DISPLAY_TIME_FM_TUNE);
	} else {
		tunerNextStation(step);
		dispMode = MODE_FM_RADIO;
		setDisplayTime(DISPLAY_TIME_FM_RADIO);
	}

	return;
}

uint8_t checkAlarmAndTime(void)
{
	uint8_t ret = ACTION_NOACTION;

	if (getClockTimer() == 0) {
		rtcReadTime();

		if (dispMode == MODE_STANDBY) {
			if ((rtc.sec == 0) &&
				(rtc.min == alarm0.min) &&
				(rtc.hour == alarm0.hour) &&
				(alarm0.wday & (0x40 >> ((rtc.wday + 5) % 7)))
				) {
				sndSetInput(alarm0.input);
				ret = ACTION_EXIT_STANDBY;
			}
		}

		setClockTimer(200);					/* Limit check interval */
	}

	return ret;
}

void handleExitDefaultMode(void)
{
	if (getDisplayTime() == 0) {
		switch (dispMode) {
		case MODE_STANDBY:
			setStbyBrightness();
			break;
		case MODE_TEMP:
			saveTempParams();
		case MODE_TEST:
			dispMode = MODE_STANDBY;
			break;
		default:
			dispMode = defDispMode();
			break;
		}
	}

	return;
}

void handleTimers(void)
{
	int16_t silenceTimer, stbyTimer;

	stbyTimer = getStbyTimer();
	silenceTimer = getSilenceTimer();

	if (dispMode != MODE_STANDBY && dispMode != MODE_TEST && dispMode != MODE_TEMP) {
		if (getSignalLevel() > 5) {
			enableSilenceTimer();
			silenceTimer = getSilenceTimer();
			if (dispMode == MODE_SILENCE_TIMER)
				dispMode = defDispMode();
		}
		if (silenceTimer >= 0 && silenceTimer < 60 && (silenceTimer < stbyTimer || stbyTimer == STBY_TIMER_OFF)) {
			dispMode = MODE_SILENCE_TIMER;
			setDisplayTime(DISPLAY_TIME_TIMER_EXP);
		}
	}
	if (stbyTimer >= 0 && stbyTimer <= 60 && stbyTimer <= silenceTimer && getDisplayTime() == 0) {
		dispMode = MODE_TIMER;
		setDisplayTime(DISPLAY_TIME_TIMER_EXP);
	}

	return;
}

void handleModeChange(void)
{
	if (dispMode != dispModePrev)
		displayClear();

	return;
}

void showScreen(void)
{
	switch (dispMode) {
	case MODE_STANDBY:
		showTime();
		break;
	case MODE_TEST:
		showRcInfo();
		break;
	case MODE_TEMP:
		showTemp();
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
		break;
	case MODE_LOUDNESS:
		showLoudness();
		break;
	case MODE_SURROUND:
		showSurround();
		break;
	case MODE_EFFECT_3D:
		showEffect3d();
		break;
	case MODE_TONE_DEFEAT:
		showToneDefeat();
		break;
	case MODE_TIME:
	case MODE_TIME_EDIT:
		showTime();
		break;
	case MODE_TIMER:
		showTimer(getStbyTimer());
		break;
	case MODE_SILENCE_TIMER:
		showTimer(getSilenceTimer());
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

	return;
}
