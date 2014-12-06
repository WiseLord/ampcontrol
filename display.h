#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "ds1307.h"
#include "audio/audio.h"

#include "display/gdfb.h"

/* Timers fo different screens */
#define DISPLAY_TIME_TEST		20000U
#define DISPLAY_TIME_TEMP		10000U
#define DISPLAY_TIME_GAIN		3000U
#define DISPLAY_TIME_TIME		3000U
#define DISPLAY_TIME_TIME_EDIT	10000U
#define DISPLAY_TIME_TIMER		5000U
#define DISPLAY_TIME_TIMER_EXP	64000U
#define DISPLAY_TIME_FM_RADIO	5000U
#define DISPLAY_TIME_FM_TUNE	10000U
#define DISPLAY_TIME_CHAN		2000U
#define DISPLAY_TIME_AUDIO		3000U
#define DISPLAY_TIME_SP			3000U
#define DISPLAY_TIME_BR			3000U

#define BACKLIGHT_ON			1
#define BACKLIGHT_OFF			0

/* Data stored in user characters */
#define LCD_LEVELS				0
#define LCD_BAR					1

/* Spectrum output mode */
enum {
	SP_MODE_STEREO,
	SP_MODE_METER,
	SP_MODE_MIXED
};

/* Display modes */
enum {
	MODE_STANDBY,
	MODE_SPECTRUM,
	MODE_FM_RADIO,
	MODE_FM_TUNE,

	MODE_VOLUME,
	MODE_BASS,
#if defined(TDA7439)
	MODE_MIDDLE,
#endif
	MODE_TREBLE,
#if defined(TDA7439)
	MODE_PREAMP,
#elif defined(TDA7313) || defined(TDA7318)
	MODE_FRONTREAR,
#endif
	MODE_BALANCE,

	MODE_GAIN,

	MODE_TIME,
	MODE_TIME_EDIT,
	MODE_TIMER,
	MODE_MUTE,
	MODE_LOUDNESS,
	MODE_TEST,

	MODE_BR,

	MODE_TEMP
};

/* Type of string printed (regular/eeprom/flash) */
#define STR_REG			0
#define STR_EEP			1
#define STR_PGM			2

#define STR_BUFSIZE		16

uint8_t getDefDisplay();

void setDefDisplay(uint8_t value);

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix);

void nextRC5Cmd(void);
void startTestMode(void);

void showRC5Info(uint8_t **txtLabels);
void showTemp(uint8_t **txtLabels);

void showRadio(uint8_t *buf, uint8_t tune);

void showMute(uint8_t **txtLabels, uint8_t *buf);
#if defined(TDA7313)
void showLoudness(uint8_t **txtLabels, uint8_t *buf);
#endif

void showBrWork(uint8_t **txtLabels, uint8_t *buf);
void changeBrWork(int8_t diff);

void showSndParam(sndParam *param, uint8_t **txtLabels, uint8_t *buf);

void showTime(uint8_t **txtLabels);
void showTimer(uint8_t *buf);
void drawSpectrum(uint8_t *buf, uint8_t **txtLabels);

void setWorkBrightness(void);
void setStbyBrightness(void);

void loadDispParams(void);
void saveDisplayParams(void);

void switchSpMode();

#endif /* DISPLAY_H */
