#include "actions.h"

#include <util/delay.h>
#include "display.h"
#include "tuner/tuner.h"
#include "input.h"

/* Leave standby mode */
void powerOn(void)
{
	PORT(STMU_STBY) |= STMU_STBY_LINE;	/* Power up audio and tuner */
	setWorkBrightness();

	_delay_ms(100);						/* Wait while power is being set up */

	tunerPowerOn();
	sndPowerOn();

	if (sndGetInput() == 0)
		tunerSetMute(MUTE_OFF);
	else
		tunerSetMute(MUTE_ON);

	return;
}

/* Entering standby mode */
void powerOff(void)
{
	sndSetMute(MUTE_ON);

	_delay_ms(100);

	PORT(STMU_STBY) &= ~STMU_STBY_LINE;

	setStbyBrightness();
	stopEditTime();
	setStbyTimer(STBY_TIMER_OFF);

	sndPowerOff();
	tunerPowerOff();
	displayPowerOff();

	return;
}

/* Next input */
void handleSetInput(uint8_t *dispMode)
{
	*dispMode = MODE_SND_GAIN0 + sndGetInput();
	setDisplayTime(DISPLAY_TIME_GAIN);

	if (*dispMode == MODE_SND_GAIN0)
		tunerSetMute(MUTE_OFF);
	else
		tunerSetMute(MUTE_ON);

	return;
}

/* Next time edit parameter */
void handleEditTime(uint8_t *dispMode)
{
	switch (*dispMode) {
	case MODE_TIME:
	case MODE_TIME_EDIT:
		editTime();
		*dispMode = MODE_TIME_EDIT;
		setDisplayTime(DISPLAY_TIME_TIME_EDIT);
		if (!isETM())
			setDisplayTime(DISPLAY_TIME_TIME);
		break;
	default:
		stopEditTime();
		*dispMode = MODE_TIME;
		setDisplayTime(DISPLAY_TIME_TIME);
		break;
	}

	return;
}

void handleSwitchMute(uint8_t *dispMode)
{
	gdClear();
	sndSetMute(!sndGetMute());
	*dispMode = MODE_MUTE;
	setDisplayTime(DISPLAY_TIME_AUDIO);

	return;
}

void handleSetDefDisplay(uint8_t *dispMode)
{
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

	return;
}

void handleSwitchSpMode(uint8_t *dispMode)
{
	switchSpMode();
	gdClear();
	*dispMode = MODE_SPECTRUM;
	setDisplayTime(DISPLAY_TIME_SP);

	return;
}

void handleSwitchFmMode(uint8_t *dispMode)
{
	switch (*dispMode) {
	case MODE_FM_RADIO:
		*dispMode = MODE_FM_TUNE;
		setDisplayTime(DISPLAY_TIME_FM_TUNE);
		break;
	case MODE_FM_TUNE:
		*dispMode = MODE_FM_RADIO;
		setDisplayTime(DISPLAY_TIME_FM_RADIO);
	}

	return;
}

void handleChangeTimer(uint8_t *dispMode, int16_t stbyTimer)
{
	stopEditTime();
	if (*dispMode == MODE_TIMER) {
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
	*dispMode = MODE_TIMER;
	setDisplayTime(DISPLAY_TIME_TIMER);

	return;
}

void handleStoreStation(uint8_t *dispMode)
{
	if (*dispMode == MODE_FM_TUNE) {
		setDisplayTime(DISPLAY_TIME_FM_TUNE);
		tunerStoreStation();
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

/* Next alarm edit parameter */
void handleEditAlarm(uint8_t *dispMode)
{
	switch (*dispMode) {
	case MODE_ALARM:
	case MODE_ALARM_EDIT:
		editAlarm();
		*dispMode = MODE_ALARM_EDIT;
		setDisplayTime(DISPLAY_TIME_ALARM_EDIT);
		if (!isEAM())
			setDisplayTime(DISPLAY_TIME_ALARM);
		break;
	default:
		stopEditAlarm();
		*dispMode = MODE_ALARM;
		setDisplayTime(DISPLAY_TIME_ALARM);
		break;
	}

	return;
}

void checkAlarm(uint8_t *dispMode)
{
	if ((getTime(DS1307_SEC) == 0) &&
	    (getTime(DS1307_MIN) == getAlarm(DS1307_A0_MIN)) &&
	    (getTime(DS1307_HOUR) == getAlarm(DS1307_A0_HOUR)) &&
	    (getAlarm(DS1307_A0_WDAY) & (0x40 >> ((getTime(DS1307_WDAY) + 5) % 7)))
	   ) {
		sndSetInput(getAlarm(DS1307_A0_INPUT));
		powerOn();
		*dispMode = MODE_TIME;
		setDisplayTime(DISPLAY_TIME_TIME);
	}

	return;
}
