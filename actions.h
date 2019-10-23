#ifndef ACTIONS_H
#define ACTIONS_H

#include <inttypes.h>

#include "audio/audio.h"
#include "input.h"

// Timers for different screens
#define DISPLAY_TIME_TEST                   20000U
#define DISPLAY_TIME_GAIN                   3000U
#define DISPLAY_TIME_GAIN_START             1500U
#define DISPLAY_TIME_TIME                   2000U
#define DISPLAY_TIME_TIME_EDIT              10000U
#define DISPLAY_TIME_FM_RADIO               5000U
#define DISPLAY_TIME_FM_TUNE                10000U
#define DISPLAY_TIME_CHAN                   2000U
#define DISPLAY_TIME_AUDIO                  3000U
#define DISPLAY_TIME_SP                     3000U
#define DISPLAY_TIME_BR                     3000U

enum {
    ACTION_EXIT_STANDBY = CMD_END,
    ACTION_ZERO_DISPLAYTIME,
    ACTION_INIT_HARDWARE,
    ACTION_TESTMODE,

    ACTION_NOACTION = 0xEF
};

// Display modes
enum {
    MODE_SPECTRUM = MODE_SND_END,
    MODE_TEST,
    MODE_STANDBY,
    MODE_TIME,
    MODE_TIME_EDIT,
    MODE_FM_RADIO,
    MODE_MUTE,
    MODE_LOUDNESS,
    MODE_SURROUND,
    MODE_EFFECT_3D,
    MODE_TONE_DEFEAT,

    MODE_BR
};

uint8_t getAction();

void handleAction(uint8_t action);
void handleEncoder(int8_t encCnt);

uint8_t checkAlarmAndTime();

void handleExitDefaultMode();
void handleModeChange();

void showScreen();

#endif // ACTIONS_H
