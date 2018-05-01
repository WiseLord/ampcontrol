#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#include "audio/audio.h"

#if defined(_KS0066_16X2_8BIT) || defined(_KS0066_16X2_PCF8574)
#define _KS0066
#endif

// Graphics (ks0108-based) or character (ks0066-based) display selection
#if !defined(_KS0108) && !defined(_KS0066)
#define _KS0108
#endif

#if defined(_KS0066)
#include "display/ks0066.h"
#define MIN_BRIGHTNESS          KS0066_MIN_BRIGHTNESS
#define MAX_BRIGHTNESS          KS0066_MAX_BRIGHTNESS
#define writeString(x)          ks0066WriteString(x)
#define displayClear()          ks0066Clear()
#else
#include "display/ks0108.h"
#define MIN_BRIGHTNESS          KS0108_MIN_BRIGHTNESS
#define MAX_BRIGHTNESS          KS0108_MAX_BRIGHTNESS
#define writeString(x)          ks0108WriteString(x)
#define displayClear()          ks0108Clear()
#endif

// Backlight state
#define BACKLIGHT_ON            1
#define BACKLIGHT_OFF           0

// Radio tuning mode
#define MODE_RADIO_TUNE         1
#define MODE_RADIO_CHAN         0

#define STR_BUFSIZE             16

// Spectrum output mode
typedef enum {
    SP_MODE_METER = 0,
    SP_MODE_STEREO,
    SP_MODE_MIXED,

    SP_MODE_END
} SpMode;

#if defined(_KS0066)
typedef enum {
    LCD_LEVELS = 0,
    LCD_BAR,

    LCD_END
} LcdSymGroup;

typedef enum {
    SYM_STEREO_DEGREE = 0,
    SYM_MUTE_CROSS,
    SYM_LOUDNESS_CROSS,
    SYM_SURROUND_CROSS,
    SYM_EFFECT_3D_CROSS,
    SYM_TONE_BYPASS_CROSS,
    SYM_STEREO_MONO,

    SYM_END
} LcdUserAddSym;
#endif

void displayInit();

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
