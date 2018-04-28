#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

// Input definitions
#define INPUT_DDR       DDRD
#define INPUT_PIN       PIND
#define INPUT_PORT      PORTD

#define BTN_5           (1<<PD7)
#define BTN_2           (1<<PD4)
#define BTN_3           (1<<PD5)
#define BTN_4           (1<<PD6)
#define BTN_1           (1<<PD0)

#define BTN_TEST_INPUT  (BTN_1 | BTN_2)
#define BTN_MASK        (BTN_1 | BTN_2 | BTN_3 | BTN_4 | BTN_5)

#define ENC_A           (1<<PD2)
#define ENC_B           (1<<PD1)
#define ENC_AB          (ENC_A | ENC_B)
#define ENC_0           0

// Standby/Mute port definitions
#define STMU_DDR        DDRC
#define STMU_PORT       PORTC

#define MUTE            (1<<PC5)
#define STDBY           (1<<PC6)

typedef enum {
    CMD_RC5_STBY,
    CMD_RC5_MUTE,
    CMD_RC_NEXT_SNDPAR,
    CMD_RC_VOL_UP,
    CMD_RC_VOL_DOWN,
    CMD_RC_IN_0,
    CMD_RC_IN_1,
    CMD_RC_IN_2,
    CMD_RC_IN_3,
    CMD_RC_IN_4,
    CMD_RC_IN_PREV,
    CMD_RC_IN_NEXT,
    CMD_RC_LOUDNESS,
    CMD_RC_SURROUND,
    CMD_RC_EFFECT_3D,
    CMD_RC_TONE_DEFEAT,

    CMD_RC_FM_RDS,              // TXT
    CMD_RC_FM_INC,
    CMD_RC_FM_DEC,
    CMD_RC_FM_MODE,             // UNIT
    CMD_RC_FM_MONO,
    CMD_RC_FM_STORE,
    CMD_RC_FM_0,
    CMD_RC_FM_1,
    CMD_RC_FM_2,
    CMD_RC_FM_3,
    CMD_RC_FM_4,
    CMD_RC_FM_5,
    CMD_RC_FM_6,
    CMD_RC_FM_7,
    CMD_RC_FM_8,
    CMD_RC_FM_9,

    CMD_RC_TIME,
    CMD_RC_ALARM,               // QUESTION
    CMD_RC_TIMER,               // TIMER
    CMD_RC_BRIGHTNESS,
    CMD_RC_DEF_DISPLAY,
    CMD_RC_NEXT_SPMODE,
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
    CMD_BTN_TESTMODE,

    CMD_END
} CmdID;

// Handling long press actions
#define SHORT_PRESS     100
#define LONG_PRESS      600

#define RC_LONG_PRESS           800
#define RC_VOL_DELAY            360
#define RC_VOL_REPEAT           400
#define RC_PRESS_LIMIT          1000

void inputInit();

int8_t getEncoder(void);
uint8_t getBtnCmd(void);
uint8_t getRcCmd(void);

void setDisplayTime(uint8_t value);
uint8_t getDisplayTime(void);

#endif // INPUT_H
