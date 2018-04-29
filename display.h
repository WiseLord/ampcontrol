#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "rtc.h"
#include "audio/audio.h"

#if defined(_KS0066_16X2_8BIT) || defined(_KS0066_16X2_PCF8574)
#define _KS0066
#endif

// Graphics (ks0108-based) or character (ks0066-based) display selection
#if !defined(_KS0108) && !defined(_KS0066) && !defined(_LS020)
#define _KS0108
#endif

#if defined(_KS0108)
#include "display/ks0108.h"
#elif defined(_KS0066)
#include "display/ks0066.h"
#elif defined(_LS020)
#include "display/ls020.h"
#endif

// Spectrum output mode
#define SP_MODE_STEREO          0
#define SP_MODE_MIXED           1

#define BACKLIGHT_ON            1
#define BACKLIGHT_OFF           0

// Data stored in user characters
#define LCD_LEVELS              0
#define LCD_BAR                 1

// Radio tuning mode
#define MODE_RADIO_TUNE         1
#define MODE_RADIO_CHAN         0

#define DISP_MIN_BR             0
#define DISP_MAX_BR             32

// Type of string printed (regular/eeprom/flash)
#define STR_REG                 0
#define STR_EEP                 1
#define STR_PGM                 2

#define STR_BUFSIZE             16

void displayInit();
void displayClear();

char *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix);

void showRCInfo();
void showRadio(uint8_t tune);
void showBoolParam(uint8_t value, uint8_t labelIndex);

void showBrWork();
void changeBrWork(int8_t diff);

void showSndParam(uint8_t mode);

void showTime();
void showSpectrum();

void setWorkBrightness(void);
void setStbyBrightness(void);

void displayPowerOff(void);

void switchSpMode();

#endif // DISPLAY_H
