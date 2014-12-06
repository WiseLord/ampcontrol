#include "actions.h"

#include <util/delay.h>
#include "display.h"
#include "tuner.h"
#include "input.h"

/* Save parameters to EEPROM */
static void saveParams(void)
{
	saveAudioParams();
	saveDisplayParams();
#if !defined(NOTUNER)
	saveTunerParams();
#endif

	return;
}

/* Leave standby mode */
static void powerOn(void)
{
	PORT(STMU_STBY) |= STMU_STBY_LINE;
	_delay_ms(50);
	setWorkBrightness();
#if !defined(NOTUNER)
	loadTunerParams();
#endif
	unmuteVolume();

	return;
}

/* Entering standby mode */
static void powerOff(void)
{
	muteVolume();
	_delay_ms(50);
	PORT(STMU_STBY) &= ~STMU_STBY_LINE;
	setStbyBrightness();
	stopEditTime();
	saveParams();

	return;
}

/* Switch power */
void handleSwitchPower(uint8_t *dispMode)
{
	switch (*dispMode) {
	case MODE_STANDBY:
		powerOn();
		*dispMode = MODE_SPECTRUM;
		break;
	default:
		powerOff();
		*dispMode = MODE_STANDBY;
		break;
	}

	return;
}

/* Next input */
void handleNextInput(uint8_t *dispMode)
{
	switch (*dispMode) {
	case MODE_GAIN:
		nextChan();
		gdClear();
	default:
		*dispMode = MODE_GAIN;
		setDisplayTime(DISPLAY_TIME_GAIN);
		break;
	}

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
	switchMute();
	*dispMode = MODE_MUTE;
	setDisplayTime(DISPLAY_TIME_AUDIO);

	return;
}

void handleNextSndParam(uint8_t *dispMode, sndParam **curSndParam)
{
	if (*dispMode >= MODE_VOLUME && *dispMode < MODE_BALANCE) {
		(*curSndParam)++;
		(*dispMode)++;
	} else {
		*curSndParam = sndParAddr(SND_VOLUME);
		*dispMode = MODE_VOLUME;
	}
	setDisplayTime(DISPLAY_TIME_AUDIO);

	return;
}

void handleSetDefDisplay(uint8_t *dispMode)
{
	switch (getDefDisplay()) {
	case MODE_SPECTRUM:
#if !defined(NOTUNER)
		if (getChan() == 0) {
			setDefDisplay(MODE_FM_RADIO);
			break;
		}
	case MODE_FM_RADIO:
#endif
		setDefDisplay(MODE_TIME);
		break;
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
