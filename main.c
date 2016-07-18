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
	setAudioParams();
	tunerSetMute(0);
	PORT(STMU_MUTE) |= STMU_MUTE_LINE;
	setWorkBrightness();
	tunerSetFreq();

	return;
}

static void powerOff(void)
{
	rtc.etm = RTC_NOEDIT;

	PORT(STMU_MUTE) &= ~STMU_MUTE_LINE;
	muteVolume();
	tunerSetMute(1);
	saveAudioParams();
	tunerPowerOff();
	setStbyBrightness();

	return;
}

static void hwInit(void)
{
	I2CInit();							// I2C bus
	ks0066Init();						// Display

	rcInit();							// RC5 IR remote control
	adcInit();							// Analog-to-digital converter
	inputInit();						// Buttons/encoder polling and timers
	tunerInit();						// Tuner

	DDR(STMU_MUTE) |= STMU_MUTE_LINE;	// Mute port
	DDR(BCKL) |= BCKL_LINE;				// Backlight port

	sei();								// Gloabl interrupt enable

	loadDispSndParams();				// Display and audio params

	powerOff();

	return;
}

int main(void)
{
	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = dispMode;

	int8_t encCnt = 0;
	uint8_t cmd = CMD_END;
	uint16_t rc5Buf = RC5_BUF_EMPTY;
	uint16_t rc5BufPrev = RC5_BUF_EMPTY;

	hwInit();

	while (1) {
		encCnt = getEncoder();
		cmd = getBtnCmd();
		rc5Buf = getRC5Buf();

		/* Poll RTC for time */
		if (getRtcTimer() == 0) {
			rtcReadTime();
			setRtcTimer(RTC_POLL_TIME);
		}

		/* Don't handle any command in test mode */
		if (dispMode == MODE_TEST) {
			if (cmd != CMD_END)
				setDispTimer(DISPLAY_TIME_TEST);
			cmd = CMD_END;
		}

		/* Don't handle any command in standby mode except power on */
		if (dispMode == MODE_STANDBY) {
			if (cmd != CMD_BTN_1 && cmd != CMD_RC_STBY &&
				cmd != CMD_BTN_12_LONG)
				cmd = CMD_END;
		}

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
		case CMD_RC_STBY:
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
		case CMD_RC_NEXT_INPUT:
			switch (dispMode) {
			case MODE_SND_GAIN0:
			case MODE_SND_GAIN1:
			case MODE_SND_GAIN2:
			case MODE_SND_GAIN3:
				nextChan();
				ks0066Clear();
			default:
				dispMode = MODE_SND_GAIN0 + getChan();
				setDispTimer(DISPLAY_TIME_GAIN);
				break;
			}
			break;
		case CMD_BTN_3:
		case CMD_RC_TIME:
			switch (dispMode) {
			case MODE_TIME:
			case MODE_TIME_EDIT:
				rtcNextEditParam();
				dispMode = MODE_TIME_EDIT;
				setDispTimer(DISPLAY_TIME_TIME_EDIT);
				if (rtc.etm == RTC_NOEDIT)
					setDispTimer(DISPLAY_TIME_TIME);
				break;
			case MODE_FM_RADIO:
				if (cmd == CMD_BTN_3) {
					tunerChangeFreq(SEARCH_DOWN);
					setDispTimer(DISPLAY_TIME_FM_RADIO);
					break;
				}
			default:
				rtc.etm = RTC_NOEDIT;
				dispMode = MODE_TIME;
				setDispTimer(DISPLAY_TIME_TIME);
				break;
			}
			break;
		case CMD_BTN_4:
		case CMD_RC_MUTE:
			switch (dispMode) {
			case MODE_FM_RADIO:
				if (cmd == CMD_BTN_4) {
					tunerChangeFreq(SEARCH_UP);
					setDispTimer(DISPLAY_TIME_FM_RADIO);
					break;
				}
			default:
				ks0066Clear();
				switchMute();
				dispMode = MODE_MUTE;
				setDispTimer(DISPLAY_TIME_CHAN);
				break;
			}
			break;
		case CMD_BTN_5:
		case CMD_RC_MENU:
			sndNextParam(&dispMode);
			setDispTimer(DISPLAY_TIME_AUDIO);
			break;
		case CMD_BTN_1_LONG:
		case CMD_RC_BACKLIGHT:
			dispMode = MODE_BR;
			setDispTimer(DISPLAY_TIME_BR);
			break;
		case CMD_BTN_2_LONG:
		case CMD_RC_DISPLAY:
			if (getChan() == 0) {
				setDispTimer(DISPLAY_TIME_FM_RADIO);
				dispMode = MODE_FM_RADIO;
			}
			break;
		case CMD_BTN_3_LONG:
		case CMD_BTN_4_LONG:
		case CMD_BTN_5_LONG:
		case CMD_RC_FM_STORE:
			if (dispMode == MODE_FM_RADIO) {
				if (cmd == CMD_BTN_3_LONG)
					tunerNextStation(SEARCH_DOWN);
				else if (cmd == CMD_BTN_4_LONG)
					tunerNextStation(SEARCH_UP);
				else
					tunerStoreStation();
				setDispTimer(DISPLAY_TIME_FM_RADIO);
			} else if (cmd == CMD_BTN_4_LONG) {
				ks0066Clear();
				switchLoudness();
				dispMode = MODE_LOUDNESS;
				setDispTimer(DISPLAY_TIME_AUDIO);
			}
			break;
		case CMD_BTN_12_LONG:
			switch (dispMode) {
			case MODE_STANDBY:
				dispMode = MODE_TEST;
				setDispTimer(DISPLAY_TIME_TEST);
				break;
			}
			break;
		case CMD_RC_LOUDNESS:
			ks0066Clear();
			switchLoudness();
			dispMode = MODE_LOUDNESS;
			setDispTimer(DISPLAY_TIME_AUDIO);
			break;
		case CMD_RC_INPUT_0:
		case CMD_RC_INPUT_1:
		case CMD_RC_INPUT_2:
		case CMD_RC_INPUT_3:
			setChan(cmd - CMD_RC_INPUT_0);
			ks0066Clear();
			dispMode = MODE_SND_GAIN0 + getChan();
			setDispTimer(DISPLAY_TIME_GAIN);
			break;
		case CMD_RC_FM_INC:
		case CMD_RC_FM_DEC:
		case CMD_RC_CHAN_UP:
		case CMD_RC_CHAN_DOWN:
			setChan(0);
			if (dispMode == MODE_FM_RADIO) {
				switch (cmd) {
				case CMD_RC_FM_INC:
					tunerChangeFreq(SEARCH_UP);
					break;
				case CMD_RC_FM_DEC:
					tunerChangeFreq(SEARCH_DOWN);
					break;
				case CMD_RC_CHAN_UP:
					tunerNextStation(SEARCH_UP);
					break;
				case CMD_RC_CHAN_DOWN:
					tunerNextStation(SEARCH_DOWN);
					break;
				}
			}
			dispMode = MODE_FM_RADIO;
			setDispTimer(DISPLAY_TIME_FM_RADIO);
			break;
		case CMD_RC_FM_MONO:
			if (getChan() == 0) {
				tunerSwitchMono();
				dispMode = MODE_FM_RADIO;
				setDispTimer(DISPLAY_TIME_FM_RADIO);
			}
			break;
		case CMD_RC_1:
		case CMD_RC_2:
		case CMD_RC_3:
		case CMD_RC_4:
		case CMD_RC_5:
		case CMD_RC_6:
		case CMD_RC_7:
		case CMD_RC_8:
		case CMD_RC_9:
		case CMD_RC_0:
			setChan(0);
			tunerLoadStation(cmd - CMD_RC_1);
			dispMode = MODE_FM_RADIO;
			setDispTimer(DISPLAY_TIME_FM_RADIO);
			break;
		}

		/* Emulate RC5 VOL_UP/VOL_DOWN as encoder actions */
		if (cmd == CMD_RC_VOL_UP)
			encCnt++;
		if (cmd == CMD_RC_VOL_DOWN)
			encCnt--;

		/* Handle encoder */
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
			case MODE_SPECTRUM:
			case MODE_TIME:
			case MODE_FM_RADIO:
				dispMode = MODE_SND_VOLUME;
			default:
				sndChangeParam(dispMode, encCnt);
				setDispTimer(DISPLAY_TIME_GAIN);
				break;
			}
		}

		/* Exit to default mode */
		if (getDispTimer() == 0) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEST:
				setStbyBrightness();
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
		switch (dispMode) {
		case MODE_STANDBY:
			showTime();
			if (dispModePrev == MODE_TEST)
				setStbyBrightness();
			break;
		case MODE_TEST:
			showRC5Info(rc5Buf);
			setWorkBrightness();
			if (rc5Buf != rc5BufPrev)
				setDispTimer(DISPLAY_TIME_TEST);
			break;
		case MODE_SPECTRUM:
			showSpectrum(getSpData());
			_delay_ms(20);
			break;
		case MODE_FM_RADIO:
			tunerReadStatus();
			showRadio();
			break;
		case MODE_MUTE:
			showBoolParam(getMute(), LABEL_MUTE);
			break;
		case MODE_LOUDNESS:
			showBoolParam(!getLoudness(), LABEL_LOUDNESS);
			break;
		case MODE_TIME:
		case MODE_TIME_EDIT:
			showTime();
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
		rc5BufPrev = rc5Buf;
	}

	return 0;
}
