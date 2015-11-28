#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

#define ENC_0					0b00
#define ENC_A					0b01
#define ENC_B					0b10
#define ENC_AB					0b11

#define BTN_STATE_0				0b00000
#define BTN_1					0b10000
#define BTN_2					0b01000
#define BTN_3					0b00100
#define BTN_4					0b00010
#define BTN_5					0b00001
#define BTN_12					0b11000
#define BTN_13					0b10100

/* RC5 support definitions */
#define RC5_CMD_COUNT			32

typedef enum {
	CMD_RC5_STBY,				// STBY
	CMD_RC5_MUTE,				// MUTE
	CMD_RC5_NEXT_SNDPAR,		// MENU
	CMD_RC5_VOL_UP,				// VOL_UP
	CMD_RC5_VOL_DOWN,			// VOL_DOWN
	CMD_RC5_IN_0,				// RED
	CMD_RC5_IN_1,				// GREEN
	CMD_RC5_IN_2,				// YELLOW
	CMD_RC5_IN_3,				// BLUE
	CMD_RC5_IN_4,
	CMD_RC5_IN_PREV,
	CMD_RC5_IN_NEXT,			// NEXT
	CMD_RC5_LOUDNESS,
	CMD_RC5_SURROUND,
	CMD_RC5_EFFECT_3D,
	CMD_RC5_TONE_DEFEAT,

	CMD_RC5_DEF_DISPLAY,		// TV
	CMD_RC5_FM_INC,				// CHAN_UP
	CMD_RC5_FM_DEC,				// CHAN_DOWN
	CMD_RC5_FM_MODE,			// UNIT
	CMD_RC5_FM_MONO,			// ASPECT
	CMD_RC5_FM_STORE,			// STORE
	CMD_RC5_FM_0,				// 0
	CMD_RC5_FM_1,				// 1
	CMD_RC5_FM_2,				// 2
	CMD_RC5_FM_3,				// 3
	CMD_RC5_FM_4,				// 4
	CMD_RC5_FM_5,				// 5
	CMD_RC5_FM_6,				// 6
	CMD_RC5_FM_7,				// 7
	CMD_RC5_FM_8,				// 8
	CMD_RC5_FM_9,				// 9

	CMD_RC5_TIME,				// TIME
	CMD_RC5_ALARM,				// QUESTION
	CMD_RC5_TIMER,				// TIMER
	CMD_RC5_BRIGHTNESS,			// PP
	CMD_RC5_NEXT_SPMODE,		// AV
	CMD_RC5_FALLSPEED,			// TXT

	CMD_RC5_END,

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

} cmdID;

/* Handling long press actions */
#define SHORT_PRESS				100
#define LONG_PRESS				600

#define STBY_TIMER_OFF			-1
#define SILENCE_TIMER			180

#define TEMP_MEASURE_TIME		2
#define SENSOR_POLL_INTERVAL	10

void rc5CodesInit(void);
void inputInit(void);

int8_t getEncoder(void);
cmdID getBtnCmd(void);

uint16_t getRC5Buf(void);
uint16_t getBtnBuf(void);
uint16_t getEncBuf(void);

void setRC5Buf(uint8_t addr, cmdID cmd);

void setDisplayTime(uint16_t value);
uint16_t getDisplayTime(void);

uint8_t getSensTimer(void);
void setSensTimer(uint8_t val);

int16_t getStbyTimer(void);
void setStbyTimer(int16_t val);

void setSecTimer(uint16_t val);
int16_t getSecTimer(void);

void setClockTimer(uint8_t value);
uint8_t getClockTimer(void);

void enableSilenceTimer(void);
void disableSilenceTimer(void);
int16_t getSilenceTimer(void);

#endif /* INPUT_H */
