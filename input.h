#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

#define RC5_SHORT_MIN 888	/* 444 microseconds */
#define RC5_SHORT_MAX 2666	/* 1333 microseconds */
#define RC5_LONG_MIN 2668	/* 1334 microseconds */
#define RC5_LONG_MAX 4444	/* 2222 microseconds */

typedef enum {
	STATE_START1,
	STATE_MID1,
	STATE_MID0,
	STATE_START0,
	STATE_ERROR,
	STATE_BEGIN,
	STATE_END
} State;


/* Initialize timer and interrupt */
void rc5Init();

/* Reset the library back to waiting-for-start state */
void rc5Reset();

/* Poll the library for new command.
 *
 * You should call RC5_Reset immediately after
 * reading the new command because it's halted once
 * receiving a full command to ensure you can read it
 * before it becomes overwritten. If you expect that only
 * one remote at a time will be used then library
 * should be polled at least once per ~150ms to ensure
 * that no command is missed.
 */
uint8_t rc5NewComm(uint16_t *newCmd);



#define RC5_STBT_MASK 0x3000
#define RC5_TOGB_MASK 0x0800
#define RC5_ADDR_MASK 0x007C
#define RC5_COMM_MASK 0x003F

#define RC5_ADDR 0x400

#define RC5_MENU 0x3B
#define RC5_VOL_UP 0x10
#define RC5_VOL_DOWN 0x11




#define BTN_DDR		DDRD
#define BTN_PORT	PORTD
#define BTN_PIN		PIND

#define BTN_MENU	(1<<PD7)
#define BTN_UP		(1<<PD4)
#define BTN_DOWN	(1<<PD5)
#define BTN_LEFT	(1<<PD6)
#define BTN_RIGHT	(1<<PD0)

#define RC5_PIN		(1<<PD3)

#define BTN_MASK (BTN_MENU | BTN_UP | BTN_DOWN | BTN_LEFT | BTN_RIGHT)

#define ENC_A		(1<<PD2)
#define ENC_B		(1<<PD1)
#define ENC_AB		(ENC_A | ENC_B)

#define COMM_ENC_UP		1
#define COMM_ENC_DOWN	2
#define COMM_BTN_MENU	3

void btnInit(void);
uint8_t getBtnComm(void);

uint8_t getRC5Comm(void);


#endif // INPUT_H
