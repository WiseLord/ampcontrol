#ifndef REMOTE_H
#define REMOTE_H

#include <inttypes.h>
#include "pins.h"

// Time scale definitions and macroses
#define RC_TIMER_DIV				4	// 1MHz / 250kHz of Timer 1 => delays in us

#define NEC_DEV_MIN					0.7
#define NEC_DEV_MAX					1.3
#define NEC_MIN(delay)				((uint16_t)(delay / RC_TIMER_DIV * NEC_DEV_MIN))
#define NEC_MAX(delay)				((uint16_t)(delay / RC_TIMER_DIV * NEC_DEV_MAX))
#define NEC_NEAR(value, delay)		(value > NEC_MIN(delay) && value < NEC_MAX(delay))

#define RC6_DEV_MIN					0.8
#define RC6_DEV_MAX					1.2
#define RC6_MIN(delay)				((uint16_t)(delay / RC_TIMER_DIV * RC6_DEV_MIN))
#define RC6_MAX(delay)				((uint16_t)(delay / RC_TIMER_DIV * RC6_DEV_MAX))
#define RC6_NEAR(value, delay)		(value > RC6_MIN(delay) && value < RC6_MAX(delay))

// Remote control types
enum {
	IR_TYPE_RC5,
	IR_TYPE_NEC,
	IR_TYPE_RC6,
	IR_TYPE_SAM,

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

// RC5/RC6 definitions
#define RC6_1T						444
#define RC6_2T						889
#define RC6_3T						1333
#define RC6_4T						1778
#define RC6_6T						2667

#define RC5_STBT_MASK				0x2000
#define RC5_FIBT_MASK				0x1000
#define RC5_TOGB_MASK				0x0800
#define RC5_ADDR_MASK				0x07C0
#define RC5_COMM_MASK				0x003F

#define RC6_ADDR_MASK				0xFFC0
#define RC6_COMM_MASK				0x00FF

typedef enum {
	STATE_RC5_MID0 = 0,
	STATE_RC5_MID1,
	STATE_RC5_START0,
	STATE_RC5_START1,
} RC5State;

// NEC/Samsung definitions
#define NEC_INIT					9000
#define SAM_INIT					4500
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

typedef union {
	uint32_t raw;
	struct {
		uint8_t laddr;
		uint8_t haddr;
		uint8_t cmd;
		uint8_t ncmd;
	};
} NECCmd;

void rcInit(void);

IRData takeIrData(void);
IRData getIrData(void);
void setIrData(uint8_t type, uint8_t addr, uint8_t cmd);

#endif /* REMOTE_H */
