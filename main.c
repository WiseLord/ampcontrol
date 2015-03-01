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
	displayInit();							/* Load params and text labels before fb scan started */
	sei();									/* Gloabl interrupt enable */

	ds18x20SearchDevices();
	tempInit();								/* Init temperature control */

	inputInit();							/* Buttons/encoder polling */

	rc5Init();								/* IR Remote control */
	adcInit();								/* Analog-to-digital converter */
	I2CInit();								/* I2C bus */


	tunerInit();							/* Tuner */

	DDR(STMU_STBY) |= STMU_STBY_LINE;		/* Standby port */
	DDR(STMU_MUTE) |= STMU_MUTE_LINE;		/* Mute port */
	sndInit();								/* Load labels/icons/etc */

	setStbyTimer(0);

	return;
}

int main(void)
{
	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = MODE_STANDBY;

	int8_t encCnt = 0;
	actionID action = ACTION_NOACTION;

	/* Init hardware */
	hwInit();

	while (1) {

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

		/* Handle encoder */
		encCnt = getEncoder();				/* Get value from encoder */
		if (action == ACTION_VOLUME_UP)		/* Emulate VOLUME_UP action as encoder action */
			encCnt++;
		if (action == ACTION_VOLUME_DOWN)	/* Emulate VOLUME_DOWN action as encoder action */
			encCnt--;
		handleEncoder(encCnt, &dispMode);

		/* Reset handled action */
		action = ACTION_NOACTION;

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
		showScreen(&dispMode, &dispModePrev);
	}

	return 0;
}
