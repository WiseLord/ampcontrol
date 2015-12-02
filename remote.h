#ifndef REMOTE_H
#define REMOTE_H

#include <inttypes.h>
#include "pins.h"

#define RC5_SHORT_MIN				1422	// 889 us ± 20%
#define RC5_SHORT_MAX				2134
#define RC5_LONG_MIN				2845	// 1778 us  ± 20%
#define RC5_LONG_MAX				4267

#define RC5_STBT_MASK				0x3000
#define RC5_TOGB_MASK				0x0800
#define RC5_ADDR_MASK				0x07C0
#define RC5_COMM_MASK				0x003F

typedef enum {
	STATE_RC5_START1,
	STATE_RC5_MID1,
	STATE_RC5_MID0,
	STATE_RC5_START0,
	STATE_RC5_ERROR,
	STATE_RC5_BEGIN,

	STATE_RC5_END
} RC5State;

#define NEC_PULSE_WIDTH_MIN			896		//  560 us ± 20%
#define NEC_PULSE_WIDTH_MAX			1344
#define NEC_ZERO_WIDTH_MIN			896		//  560 us ± 20%
#define NEC_ZERO_WIDTH_MAX			1344
#define NEC_ONE_WIDTH_MIN			2688	// 1680 us ± 20%
#define NEC_ONE_WIDTH_MAX			4032
#define NEC_START_PULSE_WIDTH_MIN	14400	// 9000 us ± 20%
#define NEC_START_PULSE_WIDTH_MAX	21600
#define NEC_START_PAUSE_WIDTH_MIN	7200	// 4500 us ± 20%
#define NEC_START_PAUSE_WIDTH_MAX	10800
#define NEC_REPEAT_WIDTH_MIN		3600	// 2250 us ± 20%
#define NEC_REPEAT_WIDTH_MAX		5400

enum {
	IR_TYPE_RC5,
	IR_TYPE_NEC,

	IR_TYPE_NONE = 0x0F
};

typedef struct {
	uint8_t ready : 1;
	uint8_t repeat : 1;
	uint8_t type : 6;
	uint8_t address;
	uint8_t command;
} IRData;

void rcInit(void);

IRData takeIrData(void);
IRData getIrData(void);
void setIrData(uint8_t type, uint8_t addr, uint8_t cmd);

#endif /* REMOTE_H */
