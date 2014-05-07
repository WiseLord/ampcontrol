#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

/* Buttons definitions */
#define BTN_DDR			DDRD
#define BTN_PIN			PIND
#define BTN_PORT		PORTD

#define BTN_5		(1<<PD7)
#define BTN_2		(1<<PD4)
#define BTN_3		(1<<PD5)
#define BTN_4		(1<<PD6)
#define BTN_1		(1<<PD0)

#define BTN_TESTMODE	(BTN_1 | BTN_4 | BTN_5)
#define BTN_MASK		(BTN_1 | BTN_2 | BTN_3 | BTN_4 | BTN_5)

/* Encoder definitions */
#define ENC_DDR			DDRD
#define ENC_PIN			PIND
#define ENC_PORT		PORTD

#define ENC_A			(1<<PD2)
#define ENC_B			(1<<PD1)
#define ENC_AB			(ENC_A | ENC_B)
#define ENC_0			0

/* Standby/Mute/Fan port definitions */
#define SMF_DDR DDRC
#define SMF_PORT PORTC

#define FAN		(1<<PC2)
#define MUTE	(1<<PC5)
#define STDBY	(1<<PC6)

enum {
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

	CMD_ENC
};

enum {
	CMD_STBY,
	CMD_MUTE,
	CMD_MENU,
	CMD_VOL_UP,
	CMD_VOL_DOWN,
	CMD_INPUT_0,
	CMD_INPUT_1,
	CMD_INPUT_2,
	CMD_INPUT_3,
	CMD_NEXT_INPUT,
	CMD_TIME,
	CMD_EDIT_TIME,
	CMD_SP_MODE,
	CMD_LOUDNESS,
	CMD_BACKLIGHT,

	CMD_EMPTY,
	CMD_TESTMODE
};

// Handling long press actions */
#define SHORT_PRESS		100
#define LONG_PRESS		600

void btnInit(void);
uint8_t getCommand(void);
void clearCommand(void);

int8_t getEncCnt(void);

void setDisplayTime(uint8_t value);
uint8_t getDisplayTime(void);

#endif /* INPUT_H */
