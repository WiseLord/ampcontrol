#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "audio/audio.h"
#include "ks0066.h"

// Timers for different screens (ms)
#define DISPLAY_TIME_TEST		15000U
#define DISPLAY_TIME_GAIN		2000U
#define DISPLAY_TIME_TIME		2000U
#define DISPLAY_TIME_TIME_EDIT	10000U
#define DISPLAY_TIME_FM_RADIO	5000U
#define DISPLAY_TIME_CHAN		2000U
#define DISPLAY_TIME_AUDIO		3000U
#define DISPLAY_TIME_SP			3000U
#define DISPLAY_TIME_BR			3000U

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
	MODE_MUTE,
	MODE_LOUDNESS,
	MODE_TEST,

	MODE_BR
};

/* Type of string printed (regular/eeprom/flash) */
#define STR_REG			0
#define STR_EEP			1
#define STR_PGM			2

#define STR_BUFSIZE		16

void showRC5Info(uint16_t rc5Buf);
void showRadio(void);
void showBoolParam(uint8_t value, uint8_t labelIndex);

void showBrWork(void);
void changeBrWork(int8_t diff);

void showSndParam(sndParam *param);

void showTime(void);
void showSpectrum(uint8_t *buf);

void setWorkBrightness(void);
void setStbyBrightness(void);

void loadDispSndParams(void);
void saveDisplayParams(void);

#endif /* DISPLAY_H */
