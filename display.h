#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>
#include "display/gdfb.h"
#include "ds1307.h"
#include "audio/audioproc.h"

/* Timers for different screens */
#define DISPLAY_TIME_TEST		20000U
#define DISPLAY_TIME_TEMP		20000U
#define DISPLAY_TIME_GAIN		3000U
#define DISPLAY_TIME_TIME		3000U
#define DISPLAY_TIME_TIME_EDIT	10000U
#define DISPLAY_TIME_TIMER		5000U
#define DISPLAY_TIME_TIMER_EXP	64000U
#define DISPLAY_TIME_ALARM		3000U
#define DISPLAY_TIME_ALARM_EDIT	10000U
#define DISPLAY_TIME_FM_RADIO	5000U
#define DISPLAY_TIME_FM_TUNE	10000U
#define DISPLAY_TIME_CHAN		2000U
#define DISPLAY_TIME_AUDIO		3000U
#define DISPLAY_TIME_SP			3000U
#define DISPLAY_TIME_BR			3000U

/* Backlight state */
#define BACKLIGHT_ON			1
#define BACKLIGHT_OFF			0

/* Radio tuning mode */
#define MODE_RADIO_TUNE			1
#define MODE_RADIO_CHAN			0

/* Spectrum output mode */
enum {
	SP_MODE_STEREO,
	SP_MODE_METER,
	SP_MODE_MIXED,
	SP_MODE_END
};

/* Display modes */
enum {
	MODE_SPECTRUM = MODE_SND_END,
	MODE_STANDBY,
	MODE_FM_RADIO,
	MODE_FM_TUNE,

	MODE_TIME,
	MODE_TIME_EDIT,
	MODE_TIMER,
	MODE_ALARM,
	MODE_ALARM_EDIT,
	MODE_MUTE,
	MODE_LOUDNESS,
	MODE_TEST,

	MODE_BR,

	MODE_TEMP
};

void displayInit(void);
uint8_t **getTxtLabels(void);

void setDefDisplay(uint8_t value);
uint8_t getDefDisplay();

void nextRC5Cmd(void);
void startTestMode(void);
void showRC5Info(void);

void showTemp(void);

void showRadio(uint8_t tune);

void showMute(void);
void showLoudness(void);

void showBrWork(void);
void changeBrWork(int8_t diff);

void showSndParam(uint8_t dispMode);

void showTime(void);
void showAlarm(void);
void showTimer(void);

void switchSpMode(void);
void showSpectrum(void);

void setWorkBrightness(void);
void setStbyBrightness(void);

void displayPowerOff(void);

#endif /* DISPLAY_H */
