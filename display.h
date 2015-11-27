#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>
#include "ds1307.h"
#include "audio/audioproc.h"

#ifdef KS0066_16X2_PCF8574
#define KS0066
#endif

#ifdef KS0066_16X2
#define KS0066
#endif

#ifdef KS0066
#include "display/ks0066.h"
#define MIN_BRIGHTNESS			KS0066_MIN_BRIGHTNESS
#define MAX_BRIGHTNESS			KS0066_MAX_BRIGHTNESS
#else
#include "display/gdfb.h"
#define MIN_BRIGHTNESS			GD_MIN_BRIGHTNESS
#define MAX_BRIGHTNESS			GD_MAX_BRIGHTNESS
#endif

/* Backlight state */
#define BACKLIGHT_ON			1
#define BACKLIGHT_OFF			0

/* Radio tuning mode */
#define MODE_RADIO_TUNE			1
#define MODE_RADIO_CHAN			0

/* String buffer */
#define STR_BUFSIZE				20

/* Spectrum output mode */
enum {
	SP_MODE_STEREO = 0,
	SP_MODE_METER,
	SP_MODE_MIXED,

	SP_MODE_END
};

enum {
	FALL_SPEED_LOW = 0,
	FALL_SPEED_MIDDLE,
	FALL_SPEED_FAST,

	FALL_SPEED_END
};

enum {
	LCD_LEVELS = 0,
	LCD_BAR,
	LCD_ALARM,

	LCD_END
};

enum {
	SYM_STEREO_DEGREE = 0,
	SYM_MUTE_CROSS,
	SYM_LOUD_CROSS,

	SYM_END
};

void displayInit(void);
void displayClear(void);

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

void showSndParam(sndMode mode);

void showTime(void);
void showAlarm(void);
void showTimer(int16_t timer);

void switchSpMode(void);
void switchFallSpeed(void);
void showSpectrum(void);

void setWorkBrightness(void);
void setStbyBrightness(void);

void displayPowerOff(void);

#endif /* DISPLAY_H */
