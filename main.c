#include <util/delay.h>
#include <avr/interrupt.h>

#include "eeprom.h"
#include "adc.h"
#include "input.h"
#include "rc5.h"
#include "i2c.h"

#include "audio.h"
#include "display.h"
#include "tuner.h"

uint8_t *txtLabels[LABELS_COUNT];	/* Array with text label pointers */
uint16_t freqFM;					/* FM freq (e.g. 10120 for 101.2MHz) */

/* Save data to EEPROM */
void saveParams(void)
{
	saveAudioParams();
	saveDisplayParams();
	saveTunerParams(freqFM);

	return;
}

/* Handle leaving standby mode */
void powerOn(void)
{
	SMF_PORT |= STDBY;
	_delay_ms(50);
	SMF_DDR |= MUTE;
	SMF_PORT |= MUTE;
	SMF_PORT |= FAN;
	loadDispParams();
	unmuteVolume();
#ifdef TUX032
	tux032ExitStby();
#endif

	return;
}

/* Handle entering standby mode */
void powerOff(void)
{
	SMF_DDR &= ~MUTE;
	SMF_PORT &= ~MUTE;
	_delay_ms(50);
	SMF_PORT &= ~STDBY;
	SMF_PORT &= ~FAN;
	DISPLAY_BACKLIGHT_PORT &= ~DISPLAY_BCKL;
	etm = NOEDIT;
	muteVolume();
	saveParams();
#ifdef TUX032
	tux032GoStby();
#endif

	return;
}

/* Hardware initialization */
void hwInit(void)
{
	loadLabels(txtLabels);			/* Load text labels from EEPROM */

	displayInit();

	rc5Init();						/* IR Remote control */
	adcInit();						/* Analog-to-digital converter */
	inputInit();					/* Buttons/encoder polling */
	I2CInit();						/* I2C bus */
	tunerInit();					/* Tuner */

	etm = NOEDIT;					/* Exit edit time mode */

	SMF_DDR |= (STDBY | FAN);		/* Standby/Mute/Fan port */
	SMF_PORT &= ~(STDBY | MUTE | FAN);

	sei();							/* Gloabl interrupt enable */

	return;
}

int main(void)
{
	hwInit();

	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = MODE_STANDBY;

	sndParam *curSndParam = &sndPar[SND_VOLUME];

	int8_t encCnt = 0;
	uint8_t cmd = CMD_EMPTY;
	uint16_t rc5Buf = RC5_BUF_EMPTY;
	uint16_t rc5BufPrev = RC5_BUF_EMPTY;
	uint8_t direction;

	loadDispParams();
	loadAudioParams(txtLabels);
	loadTunerParams(&freqFM);

	muteVolume();

	tunerSetFreq(freqFM);

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
				clearDisplay();
			default:
				curSndParam = &sndPar[SND_GAIN0 + chan];
				dispMode = MODE_GAIN;
				setDisplayTime(DISPLAY_TIME_GAIN);
				break;
			}
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
			case MODE_FM_RADIO:
				if (cmd == CMD_BTN_3) {
					tunerSetFreq(freqFM - 10);
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				}
			default:
				stopEditTime();
				dispMode = MODE_TIME;
				setDisplayTime(DISPLAY_TIME_TIME);
				break;
			}
			break;
		case CMD_BTN_4:
		case CMD_RC5_MUTE:
			switch (dispMode) {
			case MODE_FM_RADIO:
				if (cmd == CMD_BTN_4) {
					tunerSetFreq(freqFM + 10);
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
					break;
				}
			default:
				switchMute();
				dispMode = MODE_MUTE;
				setDisplayTime(DISPLAY_TIME_CHAN);
				break;
			}
			break;
		case CMD_BTN_5:
		case CMD_RC5_MENU:
			switch (dispMode) {
			case MODE_VOLUME:
			case MODE_BASS:
#ifdef TDA7439
			case MODE_MIDDLE:
#endif
			case MODE_TREBLE:
			case MODE_PREAMP:
				curSndParam++;
				dispMode++;
				break;
			default:
				curSndParam = &sndPar[SND_VOLUME];
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
					tunerSearch(freqFM, SEARCH_DOWN);
				else if (cmd == CMD_BTN_4_LONG)
					tunerSearch(freqFM, SEARCH_UP);
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
			setChan(cmd - CMD_RC5_INPUT_0);
			clearDisplay();
			curSndParam = &sndPar[SND_GAIN0 + chan];
			dispMode = MODE_GAIN;
			setDisplayTime(DISPLAY_TIME_GAIN);
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
						tunerSetFreq(freqFM + 10);
						break;
					case CMD_RC5_FM_DEC:
						tunerSetFreq(freqFM - 10);
						break;
					case CMD_RC5_CHAN_UP:
					case CMD_RC5_CHAN_DOWN:
						direction = SEARCH_UP;
						if (cmd == CMD_RC5_CHAN_DOWN)
							direction = SEARCH_DOWN;
						scanStoredFreq(freqFM, direction);
						if (!stationNum(freqFM))
							tunerSearch(freqFM, direction);
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
				curSndParam = &sndPar[SND_VOLUME];
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
				if (dispModePrev != MODE_SPECTRUM)
					saveParams();
				dispMode = MODE_SPECTRUM;
				break;
			}
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			clearDisplay();

		/* Show things */
		switch (dispMode) {
		case MODE_STANDBY:
			showTime(txtLabels);
			if (dispModePrev == MODE_TEST)
				setBacklight(0);
			break;
		case MODE_TEST:
			showRC5Info(rc5Buf);
			setBacklight(BACKLIGHT_ON);
			if (rc5Buf != rc5BufPrev)
				setDisplayTime(DISPLAY_TIME_TEST);
			break;
		case MODE_SPECTRUM:
			drawSpectrum(getSpData());
			break;
		case MODE_FM_RADIO:
			tunerReadStatus();
			freqFM = tunerFreqAvail();
			showRadio(stationNum(freqFM));
			if (tunerReady())
				fineTune(&freqFM);
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
