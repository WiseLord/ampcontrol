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
			action = checkAlarmAndTime();

		/* Convert input command to action */
		if (action == ACTION_NOACTION)
			action = getAction();

		/* Handle action */
		handleAction(action);

		/* Handle encoder */
		encCnt = getEncoder();				/* Get value from encoder */
		if (action == ACTION_VOLUME_UP)		/* Emulate VOLUME_UP action as encoder action */
			encCnt++;
		if (action == ACTION_VOLUME_DOWN)	/* Emulate VOLUME_DOWN action as encoder action */
			encCnt--;
		handleEncoder(encCnt);

		/* Reset handled action */
		action = ACTION_NOACTION;

		/* Check if we need exit to default mode*/
		handleExitDefaultMode();

		/* Switch to timer mode if it expires (less then minute) */
		handleTimerExpires();

		/* Clear screen if mode has changed */
		handleModeChange();

		/* Show things */
		showScreen();
	}

	return 0;
}
