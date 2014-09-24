#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "ds1307.h"
#include "audio/audio.h"

/* Graphics (ks0108-based) or character (ks0066-based) display selection  */
#if !defined(KS0066) && !defined(PCF8574)
#define KS0066
#endif

#if defined(KS0066)
#include "display/ks0066.h"
#elif defined(PCF8574)
#include "display/pcf8574.h"
#endif

/* Timers fo different screens */
#define DISPLAY_TIME_TEST		20
#define DISPLAY_TIME_GAIN		3
#define DISPLAY_TIME_TIME		3
#define DISPLAY_TIME_TIME_EDIT	10
#define DISPLAY_TIME_FM_RADIO	5
#define DISPLAY_TIME_CHAN		2
#define DISPLAY_TIME_MUTE		2
#define DISPLAY_TIME_AUDIO		3
#define DISPLAY_SPECTRUM		3

#define BACKLIGHT_ON			1
#define BACKLIGHT_OFF			0

/* Data stored in user characters */
#define LCD_LEVELS				0
#define LCD_BAR					1

/* Display modes */
enum {
	MODE_STANDBY,
	MODE_SPECTRUM,
	MODE_FM_RADIO,

	MODE_VOLUME,
	MODE_BASS,
	MODE_MIDDLE,
	MODE_TREBLE,
	MODE_PREAMP,
	MODE_BALANCE,

	MODE_GAIN,

	MODE_TIME,
	MODE_TIME_EDIT,
	MODE_MUTE,
	MODE_LOUDNESS,
	MODE_TEST
};

/* Type of string printed (regular/eeprom/flash) */
#define STR_REG			0
#define STR_EEP			1
#define STR_PGM			2

#define STR_BUFSIZE		16

void displayInit();

void showRC5Info(uint16_t rc5Buf);
void showRadio(uint8_t num);
void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels);

void showSndParam(sndParam *param, uint8_t **txtLabels);

void showTime(uint8_t **txtLabels);
void drawSpectrum(uint8_t *buf);

void loadDispParams(void);
void saveDisplayParams(void);

void switchBacklight(void);

#endif /* DISPLAY_H */
