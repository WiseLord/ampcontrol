#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "ds1307.h"

/* Graphics (ks0108-based) or character (ks0066-based) display selection  */
#if !defined(KS0108) && !defined(KS0066)
#define KS0066
#endif

#if defined(KS0108)
#include "display/ks0108.h"
#elif defined(KS0066)
#include "display/ks0066.h"
#endif

/* Spectrum output mode */
#define SP_MODE_STEREO			0
#define SP_MODE_MIXED			1

/* Backlight port */
#ifdef KS0108
#define DISPLAY_BACKLIGHT_DDR	GD_BACKLIGHT_DDR
#define DISPLAY_BACKLIGHT_PORT	GD_BACKLIGHT_PORT
#define DISPLAY_BCKL			GD_BCKL
#else
#define DISPLAY_BACKLIGHT_DDR	LCD_BACKLIGHT_DDR
#define DISPLAY_BACKLIGHT_PORT	LCD_BACKLIGHT_PORT
#define DISPLAY_BCKL			LCD_BCKL
#endif

/* Timers fo different screens */
#define DISPLAY_TIME_TEST		20
#define DISPLAY_TIME_GAIN		3
#define DISPLAY_TIME_TIME		3
#define DISPLAY_TIME_TIME_EDIT	10
#define DISPLAY_TIME_FM_RADIO	20
#define DISPLAY_TIME_CHAN		2
#define DISPLAY_TIME_AUDIO		3
#define DISPLAY_TIME_TEST		20

#define BACKLIGHT_ON			0
#define BACKLIGHT_OFF			1

/* Data stored in user characters */
#define LCD_LEVELS				0
#define LCD_BAR					1

enum {
	LABEL_VOLUME,
	LABEL_BASS,
	LABEL_MIDDLE, /* Loudness label for TDA7313 */
	LABEL_TREBLE,
	LABEL_PREAMP,
	LABEL_BALANCE,
	LABEL_GAIN_0,
	LABEL_GAIN_1,
	LABEL_GAIN_2,
	LABEL_GAIN_3,
	LABEL_MUTE,
	LABEL_ON,
	LABEL_OFF,
	LABEL_DB,
	LABEL_MONDAY,
	LABEL_THUESDAY,
	LABEL_WEDNESDAY,
	LABEL_THURSDAY,
	LABEL_FRIDAY,
	LABEL_SADURDAY,
	LABEL_SUNDAY
};

void displayInit();
void clearDisplay();

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix);

void showRC5Info(uint16_t rc5Buf);
void showRadio(uint8_t num);
void showParLabel(const uint8_t *parLabel, uint8_t **txtLabels);
void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels);
void showBar(int16_t min, int16_t max, int16_t value);
void showParValue(int8_t value);
void drawTm(timeMode tm, const uint8_t *font);
void showTime(uint8_t **txtLabels);
void drawSpectrum(uint8_t *buf, uint8_t mode);

#endif /* DISPLAY_H */
