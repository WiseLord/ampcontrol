#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

#define BTN_DDR			DDRD
#define BTN_PORT		PORTD
#define BTN_PIN			PIND

#define RC5_SHORT_MIN	888		/* 444 microseconds */
#define RC5_SHORT_MAX	2666	/* 1333 microseconds */
#define RC5_LONG_MIN	2668	/* 1334 microseconds */
#define RC5_LONG_MAX	4444	/* 2222 microseconds */

#define RC5_STBT_MASK	0x3000
#define RC5_TOGB_MASK	0x0800
#define RC5_ADDR_MASK	0x07C0
#define RC5_COMM_MASK	0x003F

#define RC5_ADDR		0x400

#define RC5_MENU		0x3B
#define RC5_VOL_UP		0x10
#define RC5_VOL_DOWN	0x11

#define BTN_MENU		(1<<PD7)
#define BTN_UP			(1<<PD4)
#define BTN_DOWN		(1<<PD5)
#define BTN_LEFT		(1<<PD6)
#define BTN_RIGHT		(1<<PD0)

#define RC5_PIN			(1<<PD3)

#define BTN_MASK		(BTN_MENU | BTN_UP | BTN_DOWN | BTN_LEFT | BTN_RIGHT)

#define ENC_A			(1<<PD2)
#define ENC_B			(1<<PD1)
#define ENC_AB			(ENC_A | ENC_B)
#define ENC_0			0

#define LONG_PRESS		400 /* Long press ~0.8sec while 500 polls/sec */
#define REPEAT_TIME		50

#define COMM_ENC_UP		1
#define COMM_ENC_DOWN	2
#define COMM_BTN_MENU	3

typedef enum {
	STATE_START1,
	STATE_MID1,
	STATE_MID0,
	STATE_START0,
	STATE_ERROR,
	STATE_BEGIN,
	STATE_END
} rc5State;

void rc5Init();
void rc5Reset();

void btnInit(void);
uint8_t getCommand(void);

#endif // INPUT_H
