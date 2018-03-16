#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>
#include "pins.h"

#define BTN_NO                  0x00
#define BTN_D0                  0x01
#define BTN_D1                  0x02
#define BTN_D2                  0x04
#define BTN_D3                  0x08
#define BTN_D4                  0x10
#define BTN_D5                  0x20
#define BTN_D6                  0x40
#define BTN_D7                  0x80

#define ENC_NO                  0x00
#define ENC_A                   BTN_D6
#define ENC_B                   BTN_D7
#define ENC_AB                  (ENC_A | ENC_B)

typedef enum {
    CMD_RC_STBY,                // STBY
    CMD_RC_MUTE,                // MUTE
    CMD_RC_NEXT_SNDPAR,         // MENU
    CMD_RC_VOL_UP,              // VOL_UP
    CMD_RC_VOL_DOWN,            // VOL_DOWN
    CMD_RC_IN_0,                // RED
    CMD_RC_IN_1,                // GREEN
    CMD_RC_IN_2,                // YELLOW
    CMD_RC_IN_3,                // BLUE
    CMD_RC_IN_4,
    CMD_RC_IN_PREV,
    CMD_RC_IN_NEXT,             // NEXT
    CMD_RC_LOUDNESS,
    CMD_RC_SURROUND,
    CMD_RC_EFFECT_3D,
    CMD_RC_TONE_DEFEAT,

    CMD_RC_FM_RDS,              // TXT
    CMD_RC_FM_INC,              // CHAN_UP
    CMD_RC_FM_DEC,              // CHAN_DOWN
    CMD_RC_FM_MODE,             // UNIT
    CMD_RC_FM_MONO,             // ASPECT
    CMD_RC_FM_STORE,            // STORE
    CMD_RC_FM_0,                // 0
    CMD_RC_FM_1,                // 1
    CMD_RC_FM_2,                // 2
    CMD_RC_FM_3,                // 3
    CMD_RC_FM_4,                // 4
    CMD_RC_FM_5,                // 5
    CMD_RC_FM_6,                // 6
    CMD_RC_FM_7,                // 7
    CMD_RC_FM_8,                // 8
    CMD_RC_FM_9,                // 9

    CMD_RC_TIME,                // TIME
    CMD_RC_ALARM,               // QUESTION
    CMD_RC_TIMER,               // TIMER
    CMD_RC_BRIGHTNESS,          // PP
    CMD_RC_DEF_DISPLAY,         // TV
    CMD_RC_NEXT_SPMODE,         // AV
    CMD_RC_FALLSPEED,

    CMD_RC_END,

    CMD_BTN_1,
    CMD_BTN_2,
    CMD_BTN_3,
    CMD_BTN_4,
    CMD_BTN_5,
    CMD_BTN_1_LONG,
    CMD_BTN_2_LONG,
    CMD_BTN_3_LONG,
    CMD_BTN_4_LONG,
    CMD_BTN_5_LONG,
    CMD_BTN_12_LONG,
    CMD_BTN_13_LONG,

    CMD_END

} CmdID;

enum {
    ACTION_EXIT_STANDBY = CMD_END,
    ACTION_INIT_HARDWARE,
    ACTION_TESTMODE,

    ACTION_NOACTION = 0xEF
};

// Timers
#define RTC_POLL_TIME           500

// Handling long press actions */
#define SHORT_PRESS             100
#define LONG_PRESS              600

#define RC_LONG_PRESS           800
#define RC_VOL_DELAY            360
#define RC_VOL_REPEAT           400
#define RC_PRESS_LIMIT          1000

#define INIT_TIMER_OFF          -1
#define INIT_TIMER_START        700

extern uint16_t dispTimer;
extern uint16_t rtcTimer;
extern int16_t initTimer;

void inputInit();

int8_t getEncoder();
uint8_t getBtnCmd();
uint8_t getRcCmd();

void setDispTimer(uint8_t value);

#endif // INPUT_H
