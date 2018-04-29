#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "audio/audio.h"
#include "display/ks0066.h"

// Backlight state
#define BACKLIGHT_ON            1
#define BACKLIGHT_OFF           0

// Radio tuning mode
#define MODE_RADIO_TUNE         1
#define MODE_RADIO_CHAN         0

// Data stored in user characters
#define LCD_LEVELS              0
#define LCD_BAR                 1

// Type of string printed (regular/eeprom/flash)
#define STR_REG         0
#define STR_EEP         1
#define STR_PGM         2

#define STR_BUFSIZE     16

void showRCInfo();
void showRadio(uint8_t mode);
void showBoolParam(uint8_t value, uint8_t labelIndex);

void showBrWork();
void changeBrWork(int8_t diff);

void showSndParam(uint8_t mode);

void showTime();
void showSpectrum();

void setWorkBrightness();
void setStbyBrightness();

void displayInit();
void displayPowerOff();

#endif // DISPLAY_H
