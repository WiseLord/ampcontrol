#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "ds1307.h"
#include "audio.h"

/* Graphics (ks0108-based) or character (ks0066-based) display selection  */
#if !defined(KS0108) && !defined(KS0066) && !defined(LS020)
#define KS0108
#endif

#if defined(KS0108)
#include "display/ks0108.h"
#elif defined(KS0066)
#include "display/ks0066.h"
#elif defined(LS020)
#include "display/ls020.h"
#endif

/* Spectrum output mode */
#define SP_MODE_STEREO			0
#define SP_MODE_MIXED			1

/* Backlight port */
#define DISPLAY_BCKL_DDR		DDRC
#define DISPLAY_BCKL_PORT		PORTC

#define DISPLAY_BCKL			(1<<PC7)

/* Timers fo different screens */
#define DISPLAY_TIME_TEST		20
#define DISPLAY_TIME_GAIN		3
#define DISPLAY_TIME_TIME		3
#define DISPLAY_TIME_TIME_EDIT	10
#define DISPLAY_TIME_FM_RADIO	5
#define DISPLAY_TIME_CHAN		2
#define DISPLAY_TIME_AUDIO		3
#define DISPLAY_SPECTRUM		3

#define BACKLIGHT_ON			1
#define BACKLIGHT_OFF			0

/* Data stored in user characters */
#define LCD_LEVELS				0
#define LCD_BAR					1

/* EEPROM saved labels */
enum {
	LABEL_VOLUME,
	LABEL_BASS,
#if defined(TDA7439)
	LABEL_MIDDLE,
#else
	LABEL_LOUDNESS,
#endif
	LABEL_TREBLE,
#if defined(TDA7439)
	LABEL_PREAMP,
#else
	LABEL_BALANCE_FR,
#endif
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

uint8_t getDefDisplay();
void setDefDisplay(uint8_t value);

void displayInit();
void clearDisplay();

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix);

void showRC5Info(uint16_t rc5Buf);
void showRadio(uint8_t num);
void showBoolParam(uint8_t value, const uint8_t *parLabel, uint8_t **txtLabels);

void showSndParam(sndParam *param, uint8_t **txtLabels);

void showTime(uint8_t **txtLabels);

#if defined(LS020)
void clearSpBufPrev(void);
#endif

void drawSpectrum(uint8_t *buf);

void loadDispParams(void);
void saveDisplayParams(void);

void setBacklight(int8_t backlight);
void switchBacklight(void);
void switchSpMode();

#endif /* DISPLAY_H */
