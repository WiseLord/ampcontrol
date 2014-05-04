#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

/* Buttons definitions */
#define BTN_DDR			DDRD
#define BTN_PIN			PIND
#define BTN_PORT		PORTD

#define BTN_MENU		(1<<PD7)
#define BTN_INPUT		(1<<PD4)
#define BTN_TIME		(1<<PD5)
#define BTN_MUTE		(1<<PD6)
#define BTN_STDBY		(1<<PD0)

#define BTN_TESTMODE	(BTN_STDBY | BTN_MUTE | BTN_MENU)
#define BTN_MASK		(BTN_MENU | BTN_INPUT | BTN_TIME | BTN_MUTE | BTN_STDBY)

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

typedef enum {
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
	CMD_EXT2,

	CMD_EMPTY,
	CMD_TESTMODE
} command;

// Handling long press actions */
#define SHORT_PRESS		100
#define LONG_PRESS		600

void setDisplayTime(uint16_t value);
uint16_t getDisplayTime(void);

void btnInit(void);
uint8_t getCommand(void);
uint8_t getCmdCount(void);

#endif /* INPUT_H */
