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

	setStbyTimer(0);

	return;
}

int main(void)
{
	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = MODE_STANDBY;

	int8_t encCnt = 0;
	actionID action = ACTION_NOACTION;

	hwInit();

	while (1) {
		encCnt = getEncoder();

		/* Control temperature */
		tempControlProcess();

		/* Emulate poweroff if standby timer expired */
		if (getStbyTimer() == 0)
			action = ACTION_GO_STANDBY;

		/* Check alarm and update time */
		if (action == ACTION_NOACTION)
			action = checkAlarmAndTime(&dispMode);

		/* Convert input command to action */
		if (action == ACTION_NOACTION)
			action = getAction(&dispMode);

		/* Handle action */
		handleAction(action, &dispMode);

		/* Emulate RC5 VOL_UP/VOL_DOWN as encoder actions */
		if (action == ACTION_VOLUME_UP)
			encCnt++;
		if (action == ACTION_VOLUME_DOWN)
			encCnt--;

		/* Reset handled action */
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

		/* Switch to timer mode if it expires (less then minute) */
		if (getStbyTimer() >= 0 && getStbyTimer() <= 60 && getDisplayTime() == 0) {
			dispMode = MODE_TIMER;
			setDisplayTime(DISPLAY_TIME_TIMER_EXP);
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			gdClear();


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
