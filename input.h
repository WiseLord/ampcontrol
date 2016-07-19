#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>
#include "pins.h"

#define ENC_0				0b00
#define ENC_A				0b01
#define ENC_B				0b10
#define ENC_AB				0b11

#define BTN_STATE_0			0b00000
#define BTN_1				0b10000
#define BTN_2				0b01000
#define BTN_3				0b00100
#define BTN_4				0b00010
#define BTN_5				0b00001
#define BTN_12				0b11000
#define BTN_13				0b10100

typedef enum {
	CMD_RC_STBY,			// STBY
	CMD_RC_MUTE,			// MUTE
	CMD_RC_NEXT_SNDPAR,		// MENU
	CMD_RC_VOL_UP,			// VOL_UP
	CMD_RC_VOL_DOWN,		// VOL_DOWN
	CMD_RC_IN_0,			// RED
	CMD_RC_IN_1,			// GREEN
	CMD_RC_IN_2,			// YELLOW
	CMD_RC_IN_3,			// BLUE
	CMD_RC_IN_4,
	CMD_RC_IN_PREV,
	CMD_RC_IN_NEXT,			// NEXT
	CMD_RC_LOUDNESS,
	CMD_RC_SURROUND,
	CMD_RC_EFFECT_3D,
	CMD_RC_TONE_DEFEAT,

	CMD_RC_DEF_DISPLAY,		// TV
	CMD_RC_FM_INC,			// CHAN_UP
	CMD_RC_FM_DEC,			// CHAN_DOWN
	CMD_RC_FM_MODE,			// UNIT
	CMD_RC_FM_MONO,			// ASPECT
	CMD_RC_FM_STORE,		// STORE
	CMD_RC_FM_0,			// 0
	CMD_RC_FM_1,			// 1
	CMD_RC_FM_2,			// 2
	CMD_RC_FM_3,			// 3
	CMD_RC_FM_4,			// 4
	CMD_RC_FM_5,			// 5
	CMD_RC_FM_6,			// 6
	CMD_RC_FM_7,			// 7
	CMD_RC_FM_8,			// 8
	CMD_RC_FM_9,			// 9

	CMD_RC_TIME,			// TIME
	CMD_RC_ALARM,			// QUESTION
	CMD_RC_TIMER,			// TIMER
	CMD_RC_BRIGHTNESS,		// PP
	CMD_RC_NEXT_SPMODE,		// AV
	CMD_RC_FALLSPEED,		// TXT

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

	CMD_END
} CmdID;

enum {
	ACTION_EXIT_STANDBY = CMD_END,
	ACTION_TESTMODE,

	ACTION_NOACTION = 0xEF
};

// Timers
#define RTC_POLL_TIME		500

// Handling long press actions */
#define SHORT_PRESS			100
#define LONG_PRESS			600

#define RC_LONG_PRESS		800
#define RC_VOL_DELAY		360
#define RC_VOL_REPEAT		400
#define RC_PRESS_LIMIT		1000

extern uint16_t dispTimer;
extern uint16_t rtcTimer;

void inputInit();

int8_t getEncoder(void);
uint8_t getBtnCmd(void);

void setDispTimer(uint16_t value);

#endif // INPUT_H
