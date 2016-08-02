#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "audio/audio.h"
#include "tuner/tuner.h"
#include "display.h"

#include "adc.h"
#include "eeprom.h"
#include "i2c.h"
#include "input.h"
#include "remote.h"
#include "rtc.h"

static void powerOn(void)
{
	tunerPowerOn();
	sndPowerOn();
	setWorkBrightness();
	tunerSetFreq();

	return;
}

static void powerOff(void)
{
	rtc.etm = RTC_NOEDIT;

	sndSetMute(1);
	sndPowerOff();
	tunerPowerOff();
	displayPowerOff();

	return;
}

static void hwInit(void)
{
	I2CInit();							// I2C bus
	displayInit();						// Display

	rcInit();							// RC5 IR remote control
	adcInit();							// Analog-to-digital converter
	inputInit();						// Buttons/encoder polling and timers
	tunerInit();						// Tuner

	DDR(STMU_MUTE) |= STMU_MUTE_LINE;	// Mute port
	DDR(BCKL) |= BCKL_LINE;				// Backlight port

	sei();								// Gloabl interrupt enable

	sndInit();							// Audio params, labels

	powerOff();

	return;
}

int main(void)
{
	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = dispMode;
	uint8_t fmMode = MODE_FM_RADIO;

	int8_t encCnt = 0;
	CmdID cmd;
	uint8_t action;

	hwInit();

	while (1) {
		/* Poll RTC for time */
		if (rtcTimer == 0) {
			rtcReadTime();
			rtcTimer = RTC_POLL_TIME;
		}


		/* Get command */
		cmd = getBtnCmd();

		/* If no command from buttons, get it from remote */
		if (cmd == CMD_RC_END)
			cmd = getRcCmd();

		/* Handle commands from remote control */
		if (cmd < CMD_RC_END)
			action = cmd;
		else
			action = ACTION_NOACTION;


		/* Handle commands from buttons*/
		switch (cmd) {
		case CMD_BTN_1:
			action = CMD_RC_STBY;
			break;
		case CMD_BTN_2:
			action = CMD_RC_IN_NEXT;
			break;
		case CMD_BTN_3:
			if (dispMode == MODE_FM_RADIO)
				action = CMD_RC_FM_DEC;
			else
				action = CMD_RC_TIME;
			break;
		case CMD_BTN_4:
			if (dispMode == MODE_FM_RADIO)
				action = CMD_RC_FM_INC;
			else
				action = CMD_RC_MUTE;
			break;
		case CMD_BTN_5:
			action = CMD_RC_NEXT_SNDPAR;
			break;
		case CMD_BTN_1_LONG:
			action = CMD_RC_BRIGHTNESS;
			break;
		case CMD_BTN_2_LONG:
			action = CMD_RC_DEF_DISPLAY;
			break;
		case CMD_BTN_3_LONG:
			action = CMD_RC_FM_MODE;
			break;
		case CMD_BTN_4_LONG:
			action = CMD_RC_FM_STORE;
			break;
		case CMD_BTN_12_LONG:
			action = ACTION_TESTMODE;
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


		/* Disable actions except ZERO_DISPLAY_TIME in temp mode */
		if (dispMode == MODE_TEST) {
			if (action != ACTION_NOACTION)
				setDispTimer(DISPLAY_TIME_TEST);
			action = ACTION_NOACTION;
		}
		/* Disable actions except POWERON and TESTMODE in standby mode */
		if (dispMode == MODE_STANDBY) {
			if (action != ACTION_EXIT_STANDBY && action != ACTION_TESTMODE)
				action = ACTION_NOACTION;
		}


		/* Handle command */
		switch (action) {
		case ACTION_EXIT_STANDBY:
			powerOn();
			dispMode = MODE_SPECTRUM;
			break;
		case CMD_RC_STBY:
			powerOff();
			dispMode = MODE_STANDBY;
			break;
		case CMD_RC_TIME:
			if ((dispMode == MODE_TIME || dispMode == MODE_TIME_EDIT) && rtc.etm != RTC_YEAR) {
				rtcNextEditParam();
				dispMode = MODE_TIME_EDIT;
				setDispTimer(DISPLAY_TIME_TIME_EDIT);
			} else {
				rtc.etm = RTC_NOEDIT;
				dispMode = MODE_TIME;
				setDispTimer(DISPLAY_TIME_TIME);
			}
			break;
		case CMD_RC_MUTE:
			ks0066Clear();
			sndSetMute(!aproc.mute);
			dispMode = MODE_MUTE;
			setDispTimer(DISPLAY_TIME_CHAN);
			break;
		case CMD_RC_NEXT_SNDPAR:
			sndNextParam(&dispMode);
			setDispTimer(DISPLAY_TIME_AUDIO);
			break;
		case CMD_RC_BRIGHTNESS:
			dispMode = MODE_BR;
			setDispTimer(DISPLAY_TIME_BR);
			break;
		case CMD_RC_DEF_DISPLAY:
			if (aproc.input == 0) {
				setDispTimer(DISPLAY_TIME_FM_RADIO);
				dispMode = MODE_FM_RADIO;
			}
			break;
		case ACTION_TESTMODE:
			dispMode = MODE_TEST;
			setDispTimer(DISPLAY_TIME_TEST);
			break;
		case CMD_RC_LOUDNESS:
		case CMD_RC_SURROUND:
		case CMD_RC_EFFECT_3D:
		case CMD_RC_TONE_DEFEAT:
			ks0066Clear();
			sndSwitchExtra(1 << (action - CMD_RC_LOUDNESS));
			dispMode = MODE_LOUDNESS + (action - CMD_RC_LOUDNESS);
			setDispTimer(DISPLAY_TIME_AUDIO);
			break;
		case CMD_RC_IN_0:
		case CMD_RC_IN_1:
		case CMD_RC_IN_2:
		case CMD_RC_IN_3:
		case CMD_RC_IN_4:
			ks0066Clear();
			sndSetInput(action - CMD_RC_IN_0);
			dispMode = MODE_SND_GAIN0 + aproc.input;
			setDispTimer(DISPLAY_TIME_GAIN);
			break;
		default:
			if (!aproc.input && tuner.ic) {
				if (action >= CMD_RC_FM_INC && action <= CMD_RC_FM_9) {
					if (dispMode != MODE_FM_RADIO)
						fmMode = MODE_RADIO_CHAN;
					dispMode = MODE_FM_RADIO;
					setDispTimer(DISPLAY_TIME_FM_RADIO);
				}
				switch (action) {
				case CMD_RC_FM_MODE:
					fmMode = !fmMode;
					break;
				case CMD_RC_FM_INC:
					tunerNextStation(SEARCH_UP);
					break;
				case CMD_RC_FM_DEC:
					tunerNextStation(SEARCH_DOWN);
					break;
				case CMD_RC_FM_STORE:
					tunerStoreStation();
					break;
				case CMD_RC_FM_MONO:
					tunerSetMono(!tuner.mono);
					break;
#ifdef _RDS
				case CMD_RC_FM_RDS:
					tunerSetRDS(!tuner.rds);
#endif
				default:
					if (action >= CMD_RC_FM_0 && action <= CMD_RC_FM_9) {
						sndSetInput(0);
						tunerLoadStation(action - CMD_RC_FM_0);
					}
					break;
				}
			}
			break;
		}


		/* Handle encoder */
		encCnt = getEncoder();

		/* Emulate RC5 VOL_UP/VOL_DOWN as encoder actions */
		if (action == CMD_RC_VOL_UP)
			encCnt++;
		if (action == CMD_RC_VOL_DOWN)
			encCnt--;

		if (encCnt) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEST:
				setDispTimer(DISPLAY_TIME_TEST);
				break;
			case MODE_TIME_EDIT:
				rtcChangeTime(encCnt);
				setDispTimer(DISPLAY_TIME_TIME_EDIT);
				break;
			case MODE_BR:
				changeBrWork(encCnt);
				setDispTimer(DISPLAY_TIME_BR);
				break;
			case MODE_FM_RADIO: {
				if (fmMode == MODE_RADIO_TUNE) {
					tunerChangeFreq(encCnt);
					setDispTimer(DISPLAY_TIME_FM_RADIO);
					break;
				}
			}
			case MODE_SPECTRUM:
			case MODE_TIME:
			case MODE_MUTE:
			case MODE_LOUDNESS:
				dispMode = MODE_SND_VOLUME;
			default:
				sndSetMute(0);
				sndChangeParam(dispMode, encCnt);
				setDispTimer(DISPLAY_TIME_GAIN);
				break;
			}
		}


		/* Exit to default mode */
		if (dispTimer == 0) {
			switch (dispMode) {
			case MODE_STANDBY:
			case MODE_TEST:
				dispMode = MODE_STANDBY;
				break;
			default:
				dispMode = MODE_SPECTRUM;
				break;
			}
		}


		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			ks0066Clear();


		/* Show things */
		ks0066SetXY(0, 0);
		switch (dispMode) {
		case MODE_STANDBY:
			setStbyBrightness();
		case MODE_TIME:
		case MODE_TIME_EDIT:
			showTime();
			break;
		case MODE_TEST:
			setWorkBrightness();
			showRC5Info();
			break;
		case MODE_SPECTRUM:
			showSpectrum();
			break;
		case MODE_FM_RADIO:
			showRadio(fmMode);
			break;
		case MODE_MUTE:
			showBoolParam(aproc.mute, LABEL_MUTE);
			break;
		case MODE_LOUDNESS:
		case MODE_SURROUND:
		case MODE_EFFECT_3D:
		case MODE_TONE_DEFEAT:
			showBoolParam(aproc.extra & (1 << (dispMode - MODE_LOUDNESS)), LABEL_LOUDNESS + (dispMode - MODE_LOUDNESS));
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
	}

	return 0;
}
