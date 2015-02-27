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
#define RC5_CMD_COUNT	32

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
	CMD_RC5_FALLSPEED,
	CMD_RC5_DISPLAY,
	CMD_RC5_TIMER,
	CMD_RC5_ALARM,

	CMD_RC5_FM_CHAN_UP,
	CMD_RC5_FM_CHAN_DOWN,
	CMD_RC5_FM_TUNE,
	CMD_RC5_FM_MONO,
	CMD_RC5_FM_STORE,
	CMD_RC5_FM_1,
	CMD_RC5_FM_2,
	CMD_RC5_FM_3,
	CMD_RC5_FM_4,
	CMD_RC5_FM_5,
	CMD_RC5_FM_6,
	CMD_RC5_FM_7,
	CMD_RC5_FM_8,
	CMD_RC5_FM_9,
	CMD_RC5_FM_0,

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
	CMD_BTN_TEMP,

	CMD_EMPTY = 0xEF
};

/* Handling long press actions */
#define SHORT_PRESS		100
#define LONG_PRESS		600

#define STBY_TIMER_OFF	-1

void inputInit();

int8_t getEncoder(void);
uint8_t getBtnCmd(void);

uint16_t getRC5Buf(void);
uint16_t getBtnBuf(void);
uint16_t getEncBuf(void);

void setRC5Buf(uint8_t addr, uint8_t cmd);

void setDisplayTime(uint16_t value);
uint16_t getDisplayTime(void);

uint16_t getTempTimer(void);
void setTempTimer(uint16_t val);

int16_t getStbyTimer(void);
void setStbyTimer(int16_t val);

void setSecTimer(uint16_t val);

void setClockTimer(uint8_t value);
uint8_t getClockTimer(void);

#endif /* INPUT_H */
