#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>
#include "pins.h"

#define ENC_0			0b00
#define ENC_A			0b01
#define ENC_B			0b10
#define ENC_AB			0b11

#define BTN_STATE_0		0b00000
#define BTN_1			0b10000
#define BTN_2			0b01000
#define BTN_3			0b00100
#define BTN_4			0b00010
#define BTN_5			0b00001
#define BTN_12			0b11000
#define BTN_13			0b10100

typedef enum {
	CMD_RC_STBY,
	CMD_RC_MUTE,
	CMD_RC_MENU,
	CMD_RC_VOL_UP,
	CMD_RC_VOL_DOWN,
	CMD_RC_IN_0,
	CMD_RC_IN_1,
	CMD_RC_IN_2,
	CMD_RC_IN_3,
	CMD_RC_IN_NEXT,
	CMD_RC_TIME,
	CMD_RC_BACKLIGHT,
	CMD_RC_SP_MODE,
	CMD_RC_LOUDNESS,
	CMD_RC_CHAN_UP,
	CMD_RC_CHAN_DOWN,

	CMD_RC_FM_INC,
	CMD_RC_FM_DEC,
	CMD_RC_FM_STORE,
	CMD_RC_1,
	CMD_RC_2,
	CMD_RC_3,
	CMD_RC_4,
	CMD_RC_5,
	CMD_RC_6,
	CMD_RC_7,
	CMD_RC_8,
	CMD_RC_9,
	CMD_RC_0,
	CMD_RC_DISPLAY,
	CMD_RC_FM_MONO,

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
#define RTC_POLL_TIME	1000

// Handling long press actions */
#define SHORT_PRESS		100
#define LONG_PRESS		600

#define RC_LONG_PRESS	800
#define RC_VOL_DELAY	360
#define RC_VOL_REPEAT	400
#define RC_PRESS_LIMIT	1000

void inputInit();

int8_t getEncoder(void);
uint8_t getBtnCmd(void);
uint16_t getRC5Buf(void);

uint16_t getDispTimer(void);
void setDispTimer(uint16_t value);

uint16_t getRtcTimer(void);
void setRtcTimer(uint16_t val);

#endif /* INPUT_H */
