#ifndef REMOTE_H
#define REMOTE_H

#include <inttypes.h>
#include "pins.h"

#define RC5_SHORT_MIN	888		/* 444 microseconds */
#define RC5_SHORT_MAX	2666	/* 1333 microseconds */
#define RC5_LONG_MIN	2668	/* 1334 microseconds */
#define RC5_LONG_MAX	4444	/* 2222 microseconds */

#define RC5_STBT_MASK	0x3000
#define RC5_TOGB_MASK	0x0800
#define RC5_ADDR_MASK	0x07C0
#define RC5_COMM_MASK	0x003F

typedef enum {
	STATE_RC5_START1,
	STATE_RC5_MID1,
	STATE_RC5_MID0,
	STATE_RC5_START0,
	STATE_RC5_ERROR,
	STATE_RC5_BEGIN,

	STATE_RC5_END
} RC5State;

enum {
	IR_TYPE_NONE = 0,
	IR_TYPE_RC5,
	IR_TYPE_NEC,

	IR_TYPE_END
};

typedef struct {
	uint8_t type : 7;
	uint8_t repeat : 1;
	uint8_t address;
	uint8_t command;
} IRData;

void rcInit(void);

IRData takeIRData(void);
IRData getIrData(void);
void setIrData(uint8_t addr, uint8_t cmd);

#endif /* REMOTE_H */
