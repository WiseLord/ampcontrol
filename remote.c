#include "remote.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static volatile IRData irData;						// Last decoded IR command
static volatile uint8_t ovfCnt = 250;				// Overflow counter

void rcInit(void)
{
	MCUCR |= (1<<ISC10);							// Set INT1 to trigger on any edge
	DDR(RC) &= ~RC_LINE;							// Set PD3 (INT1) to input
	TCCR1A = 0;										// Reset Timer1 counter
	TCCR1B = (1<<CS11) | (1<<CS10);					// Set Timer1 prescaler to 64 (250kHz)
	GICR |= (1<<INT1);								// Enable INT3 interrupt

	TIMSK = (1<<TOIE1);								// Enable Timer1 overflow interrupt

	return;
}

ISR(TIMER1_OVF_vect)								// Overflow every 1/(250kHz/65536) = 262ms
{
	if (ovfCnt <= 250)
		ovfCnt++;

	return;
}

ISR(INT1_vect)
{
	static uint16_t tcnt = 0;
	uint16_t delay = TCNT1 - tcnt;					// Delay since previous interrupt

	tcnt = TCNT1;

	// RC pin state on interrupt event is inverted due to inverted IR receiver polarity
	uint8_t rcPin = !(PIN(RC) & RC_LINE);

	// NEC protocol variables
	static uint8_t necCnt = 0;						// NEC bit counter
	static NECCmd necCmd;							// NEC command
	static NECState necState = STATE_NEC_IDLE;		// NEC decoding state

	// RC5/RC6 protocol variables
	static uint8_t rc5Cnt = 16;						// RC5 bit counter
	static uint16_t rc5Cmd = 0;						// RC5 command
	static RC5State rc5State = STATE_RC5_START1;	// RC5 decoding state
	static int8_t rc6Cnt = 0;						// RC6 bit counter
	static uint16_t rc6Cmd = 0;						// RC6 command
	static RC5State rc6State = STATE_RC5_START1;	// RC6 decoding state

	static uint8_t rc6TogBitOld = 0;
	uint8_t rc6TogBit = 0;

	if (rcPin) {
		// Try to decode as NEC sequence
		if (necState == STATE_NEC_INIT) {
			if (RC_NEAR(delay, NEC_START)) {
				necState = STATE_NEC_RECEIVE;
			} else if (RC_NEAR(delay, NEC_REPEAT) && ovfCnt < 2) {
				irData.repeat = 1;
				irData.ready = 1;
				ovfCnt = 0;
			}
			necCnt = 0;
		} else if (necState == STATE_NEC_RECEIVE) {
			necCnt++;
			necCmd.raw >>= 1;
			if (RC_NEAR(delay, NEC_ZERO))
				necCmd.raw &= ~0x80000000;
			else if (RC_NEAR(delay, NEC_ONE))
				necCmd.raw |= 0x80000000;
			else
				necCnt = 0;
			if (necCnt == 32) {
				if ((uint8_t)(~necCmd.ncmd) == necCmd.cmd) {
					irData.ready = 1;
					if (ovfCnt < 2)
						irData.repeat = 1;
					else
						irData.repeat = 0;
					irData.address = necCmd.laddr;
					irData.command = necCmd.cmd;
					ovfCnt = 0;
				}
			}
		}
		// Try to decode as RC6 sequence
		if (RC_NEAR(delay, RC6_1T)) {
			if (rc6State == STATE_RC5_START0) {
				rc6State = STATE_RC5_MID0;
				if (--rc6Cnt < 16)
					rc6Cmd <<= 1;
			} else if (rc6State == STATE_RC5_MID1) {
				rc6State = STATE_RC5_START1;
			}
			rc5Cnt = 13;								// Reset
			rc5State = STATE_RC5_MID1;
		} else if (RC_NEAR(delay, RC6_2T)) {
			if (rc6State == STATE_RC5_MID1) {
				if (rc6Cnt == 21 || rc6Cnt == 16) {
					rc6State = STATE_RC5_START1;
				} else {
					rc6State = STATE_RC5_MID0;
					if (--rc6Cnt < 16)
						rc6Cmd <<= 1;
				}
			} else if (rc6State == STATE_RC5_START0) {
				if (rc6Cnt == 17) {
					rc6State = STATE_RC5_MID0;
					--rc6Cnt;
					rc6TogBit = 0;
				}
			}
			if (rc5State == STATE_RC5_START1) {
				rc5State = STATE_RC5_MID1;
				rc5Cnt--;
				rc5Cmd <<= 1;
				rc5Cmd |= 0x01;
			} else if (rc5State == STATE_RC5_MID0) {
				rc5State = STATE_RC5_START0;
			}
		} else if (RC_NEAR(delay, RC6_3T)) {
			if (rc6State == STATE_RC5_MID1) {
				if (rc6Cnt == 17) {
					rc6State = STATE_RC5_MID0;
					--rc6Cnt;
					rc6TogBit = 0;
				} else if (rc6Cnt == 21 || rc6Cnt == 16) {
					rc6State = STATE_RC5_MID0;
					if (--rc6Cnt < 16) {
						rc6Cmd <<= 1;
					}
				}
			}
			rc5Cnt = 13;								// Reset
			rc5State = STATE_RC5_MID1;
		} else if (RC_NEAR(delay, RC6_4T)) {
			if (rc5State == STATE_RC5_MID0) {
				rc5State = STATE_RC5_MID1;
				rc5Cnt--;
				rc5Cmd <<= 1;
				rc5Cmd |= 0x01;
			}
		} else {
			rc6Cnt = 22;								// Reset
			rc6State = STATE_RC5_START1;
			rc5Cnt = 13;								// Reset
			rc5State = STATE_RC5_MID1;
		}
	} else {
		// Try to decode as NEC sequence
		if (RC_NEAR(delay, NEC_PULSE) && necState != STATE_NEC_REPEAT) {
			necState = STATE_NEC_RECEIVE;
		} else if (RC_NEAR(delay, NEC_INIT)) {
			necState = STATE_NEC_INIT;
			irData.type = IR_TYPE_NEC;
		} else if (RC_NEAR(delay, SAM_INIT)) {
			necState = STATE_NEC_INIT;
			irData.type = IR_TYPE_SAM;
		} else {
			necState = STATE_NEC_IDLE;
		}
		// Try to decode as RC6 sequence
		if (RC_NEAR(delay, RC6_1T)) {
			if (rc6State == STATE_RC5_START1) {
				rc6State = STATE_RC5_MID1;
				if (--rc6Cnt < 16) {
					rc6Cmd <<= 1;
					rc6Cmd |= 0x01;
				}
			} else if (rc6State == STATE_RC5_MID0) {
				rc6State = STATE_RC5_START0;
			}
		} else if (RC_NEAR(delay, RC6_2T)) {
			if (rc6State == STATE_RC5_MID0) {
				if (rc6Cnt == 16) {
					rc6State = STATE_RC5_START0;
				} else {
					rc6State = STATE_RC5_MID1;
					if (--rc6Cnt < 16) {
						rc6Cmd <<= 1;
						rc6Cmd |= 0x01;
					}
				}
			} else if (rc6State == STATE_RC5_START1) {
				if (rc6Cnt == 17) {
					rc6State = STATE_RC5_MID1;
					--rc6Cnt;
					rc6TogBit = 1;
				}
			}
			if (rc5State == STATE_RC5_MID1) {
				rc5State = STATE_RC5_START1;
			} else if (rc5State == STATE_RC5_START0) {
				rc5State = STATE_RC5_MID0;
				rc5Cnt--;
				rc5Cmd <<= 1;
			}
		} else if (RC_NEAR(delay, RC6_3T)) {
			if (rc6State == STATE_RC5_MID0) {
				if (rc6Cnt == 17) {
					rc6State = STATE_RC5_MID1;
					--rc6Cnt;
					rc6TogBit = 1;
				} else if (rc6Cnt == 16) {
					rc6State = STATE_RC5_MID1;
					rc6Cmd <<= 1;
					rc6Cmd |= 0x01;
				}
			}
		} else if (RC_NEAR(delay, RC6_4T)) {
			if (rc5State == STATE_RC5_MID1) {
				rc5State = STATE_RC5_MID0;
				rc5Cnt--;
				rc5Cmd <<= 1;
			}
		} else if  (RC_NEAR(delay, RC6_6T)) {
			rc6State = STATE_RC5_MID1;
			rc6Cnt = 21;
		} else {
			rc6Cnt = 22;								// Reset
			rc6State = STATE_RC5_START1;
		}
	}

	if (rc5Cnt == 0 || rc6Cnt == 0) {
		if (rc5Cnt == 0) {
			rc6TogBit = (rc5Cmd & RC5_TOGB_MASK) != 0;
			irData.type = IR_TYPE_RC5;
			irData.address  = (rc5Cmd & RC5_ADDR_MASK) >> 6;
			irData.command = (rc5Cmd & RC5_COMM_MASK) | (rc5Cmd & RC5_FIBT_MASK ? 0x00 : 0x40);
		} else {
			irData.type = IR_TYPE_RC6;
			irData.address  = (rc6Cmd & RC6_ADDR_MASK) >> 8;
			irData.command = rc6Cmd & RC6_COMM_MASK;
		}
		rc6Cnt = 22;
		rc5Cnt = 13;
		irData.ready = 1;
		irData.repeat = (rc6TogBit == rc6TogBitOld);
		rc6TogBitOld = rc6TogBit;
	}
	return;
}

IRData takeIrData()
{
	IRData ret = irData;
	irData.ready = 0;

	return ret;
}

IRData getIrData()
{
	return irData;
}

void setIrData(uint8_t type, uint8_t addr, uint8_t cmd)
{
	irData.type = type;
	irData.address = addr;
	irData.command = cmd;

	return;
}
