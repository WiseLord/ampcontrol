#ifndef RC5_H
#define RC5_H

#include <inttypes.h>

#define RC5_DDR			DDRD
#define RC5_PIN			PIND
#define RC5_PORT		PORTD

#define RC5_DATA		(1<<PD3)

#define RC5_SHORT_MIN	444		/* 444 microseconds */
#define RC5_SHORT_MAX	1333	/* 1333 microseconds */
#define RC5_LONG_MIN	1334	/* 1334 microseconds */
#define RC5_LONG_MAX	2222	/* 2222 microseconds */

#define RC5_STBT_MASK	0x3000
#define RC5_TOGB_MASK	0x0800
#define RC5_ADDR_MASK	0x07C0
#define RC5_COMM_MASK	0x003F

#define RC5_BUF_EMPTY	0

typedef enum {
	STATE_START1,
	STATE_MID1,
	STATE_MID0,
	STATE_START0,
	STATE_ERROR,
	STATE_BEGIN,
	STATE_END
} rc5State;

void rc5Init(void);

uint16_t getRC5RawBuf(void);

#endif // RC5_H
