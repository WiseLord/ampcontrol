#ifndef RC5_H
#define RC5_H

#include <inttypes.h>
#include "pins.h"

#define RC5_SHORT_MIN	444		/* 444 microseconds */
#define RC5_SHORT_MAX	1333	/* 1333 microseconds */
#define RC5_LONG_MIN	1334	/* 1334 microseconds */
#define RC5_LONG_MAX	2222	/* 2222 microseconds */

#define RC5_STBT_MASK	0x3000
#define RC5_TOGB_MASK	0x0800
#define RC5_ADDR_MASK	0x07C0
#define RC5_COMM_MASK	0x003F

#define RC5_BUF_EMPTY	0

void rcInit(void);

uint16_t getRCRawBuf(void);

#endif /* RC5_H */
