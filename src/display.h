#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>
#include "audio/audio.h"

#if defined(_KS0066_16X2_4BIT) || defined(_KS0066_16X2_PCF8574)
#define _KS0066
#endif

#if defined(_KS0066)
#include "display/ks0066.h"
#define MIN_BRIGHTNESS          KS0066_MIN_BRIGHTNESS
#define MAX_BRIGHTNESS          KS0066_MAX_BRIGHTNESS
#define writeData(x)            ks0066WriteData(x)
#define writeString(x)          ks0066WriteString(x)
#define displayClear()          ks0066Clear()
#endif

// Backlight state
#define BACKLIGHT_ON            1
#define BACKLIGHT_OFF           0

// Radio tuning mode
#define MODE_RADIO_TUNE         1
#define MODE_RADIO_CHAN         0

// String buffer
#define STR_BUFSIZE     16

#if defined(_KS0066)
typedef enum {
    LCD_LEVELS = 0,
    LCD_BAR,

    LCD_END
} LcdSymGroup;
#endif

void displayInit();
void displayPowerOff();

void changeBrWork(int8_t diff);
void setWorkBrightness();
void setStbyBrightness();

void showBrWork();
void showRadio(uint8_t mode);
void showRCInfo();
void showTime();
void showSpectrum();

void showBoolParam(uint8_t value, uint8_t labelIndex);

void showSndParam(sndMode mode);

#endif // DISPLAY_H
