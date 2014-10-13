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

/* RC5 support definitions */
#define RC5_CMD_COUNT	31

enum {
	CMD_RC5_STBY,
	CMD_RC5_MUTE,
	CMD_RC5_MENU,
	CMD_RC5_VOL_UP,
	CMD_RC5_VOL_DOWN,
	CMD_RC5_INPUT_0,
	CMD_RC5_INPUT_1,
	CMD_RC5_INPUT_2,
	CMD_RC5_INPUT_3,
	CMD_RC5_NEXT_INPUT,
	CMD_RC5_TIME,
	CMD_RC5_BACKLIGHT,
	CMD_RC5_SP_MODE,
	CMD_RC5_LOUDNESS,
	CMD_RC5_CHAN_UP,
	CMD_RC5_CHAN_DOWN,
	CMD_RC5_FM_INC,
	CMD_RC5_FM_DEC,
	CMD_RC5_FM_STORE,
	CMD_RC5_1,
	CMD_RC5_2,
	CMD_RC5_3,
	CMD_RC5_4,
	CMD_RC5_5,
	CMD_RC5_6,
	CMD_RC5_7,
	CMD_RC5_8,
	CMD_RC5_9,
	CMD_RC5_0,
	CMD_RC5_DISPLAY,
	CMD_RC5_FM_MONO,

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
	CMD_BTN_TEST,

	CMD_EMPTY = 0xEF
};

// Handling long press actions */
#define SHORT_PRESS		100
#define LONG_PRESS		600

void inputInit();

int8_t getEncoder(void);
uint8_t getBtnCmd(void);
uint16_t getRC5Buf(void);

void setDisplayTime(uint8_t value);
uint8_t getDisplayTime(void);

#endif /* INPUT_H */
