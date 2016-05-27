#ifndef REMOTE_H
#define REMOTE_H

#include <inttypes.h>
#include "pins.h"

// Time scale definitions and macroses
#define RC_TIMER_DIV				4	// 1MHz / 250kHz of Timer 1 => delays in us
#define RC_DEV_MIN					0.8
#define RC_DEV_MAX					1.2
#define RC_MIN(delay)				((uint16_t)(delay / RC_TIMER_DIV * RC_DEV_MIN))
#define RC_MAX(delay)				((uint16_t)(delay / RC_TIMER_DIV * RC_DEV_MAX))
#define RC_NEAR(value, delay)		(value > RC_MIN(delay) && value < RC_MAX(delay))

// Remote control types
enum {
	IR_TYPE_RC5,
	IR_TYPE_NEC,

	IR_TYPE_NONE = 0x0F
};

// Structure for strore data received
typedef struct {
	uint8_t ready : 1;
	uint8_t repeat : 1;
	uint8_t rsvd1 : 1;
	uint8_t rsvd2 : 1;
	uint8_t type : 4;
	uint8_t address;
	uint8_t command;
} IRData;

// RC5 definitions
#define RC5_SHORT					889
#define RC5_LONG					1778

#define RC5_STBT_MASK				0x3000
#define RC5_TOGB_MASK				0x0800
#define RC5_ADDR_MASK				0x07C0
#define RC5_COMM_MASK				0x003F

typedef enum {
	EVENT_RC5_SHORT_SPACE = 0,
	EVENT_RC5_SHORT_PULSE = 2,
	EVENT_RC5_LONG_SPACE  = 4,
	EVENT_RC5_LONG_PULSE  = 6,
} RC5Event;

typedef enum {
	STATE_RC5_START1 = 0,
	STATE_RC5_MID1   = 1,
	STATE_RC5_MID0   = 2,
	STATE_RC5_START0 = 3,
} RC5State;

// NEC definitions
#define NEC_INIT					9000
#define NEC_START					4500
#define NEC_REPEAT					2250
#define NEC_ZERO					560
#define NEC_ONE						1680
#define NEC_PULSE					560

typedef enum {
	STATE_NEC_IDLE = 0,
	STATE_NEC_INIT,
	STATE_NEC_REPEAT,
	STATE_NEC_RECEIVE,
} NECState;

void rcInit(void);

IRData takeIrData(void);
IRData getIrData(void);
void setIrData(uint8_t type, uint8_t addr, uint8_t cmd);

#endif /* REMOTE_H */
