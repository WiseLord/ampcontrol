#include "remote.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static volatile IRData irData;						// Last decoded IR command

void rcInit(void)
{
	MCUCR |= (1<<ISC10);							// Set INT1 to trigger on any edge
	DDR(RC) &= ~RC_LINE;							// Set PD3 (INT1) to input
	TCCR1A = 0;										// Reset Timer1 counter
	TCCR1B = (1<<CS11) | (1<<CS10);					// Set Timer1 prescaler to 64 (250kHz)
	GICR |= (1<<INT1);								// Enable INT3 interrupt

	return;
}

ISR(INT1_vect)
{
	// RC pin state on interrupt event is inverted due to inverted IR receiver polarity
	uint8_t rcPin = !(PIN(RC) & RC_LINE);

	static uint16_t tcnt = 0;

	uint16_t delay = TCNT1 - tcnt;					// Delay since previous interrupt
	tcnt = TCNT1;

	// ===============================================================================
	// Try to decode as NEC sequence
	// ===============================================================================

	static uint8_t necCnt = 0;						// NEC bit counter
	static uint32_t necCmd = 0;						// NEC command
	static NECState necState = STATE_NEC_IDLE;		// NEC decoding state

	if (rcPin) {
		if (necState == STATE_NEC_INIT) {
			if (RC_NEAR(delay, NEC_START)) {
				necState = STATE_NEC_RECEIVE;
			} else if (RC_NEAR(delay, NEC_REPEAT)) {
				irData.repeat = 1;
				irData.ready = 1;
			}
			necCnt = 0;
		} else if (necState == STATE_NEC_RECEIVE) {
			necCnt++;
			necCmd >>= 1;
			if (RC_NEAR(delay, NEC_ZERO))
				necCmd &= ~0x80000000;
			else if (RC_NEAR(delay, NEC_ONE))
				necCmd |= 0x80000000;
			else
				necCnt = 0;
			if (necCnt == 32) {
				irData.ready = 1;
				irData.repeat = 0;
				irData.type = IR_TYPE_NEC;
				irData.address = necCmd & 0xFF;
				irData.command = (necCmd >> 16) & 0xFF;
			}
		}
	} else {
		if (RC_NEAR(delay, NEC_PULSE) && necState != STATE_NEC_REPEAT)
			necState = STATE_NEC_RECEIVE;
		else if (RC_NEAR(delay, NEC_INIT))
			necState = STATE_NEC_INIT;
		else
			necState = STATE_NEC_IDLE;
	}

	// ===============================================================================
	// Try to decode as RC5 sequence
	// ===============================================================================

	// State machine transition table
	static const uint8_t trans[4] PROGMEM = {0x01, 0x91, 0x9b, 0xfb};

	static RC5State rc5State = STATE_RC5_MID1;		// RC5 decoding state
	static uint16_t rc5Cmd = 0;						// RC5 command
	static uint8_t rc5Cnt = 0;						// RC5 bit counter
	static uint8_t rc5TogBitOld = 0;

	RC5State newstate;
	RC5Event event;
	uint8_t rc5TogBit;

	event = (rcPin ? EVENT_RC5_SHORT_SPACE : EVENT_RC5_SHORT_PULSE);

	if (RC_NEAR(delay, RC5_LONG)) {
		event += 4;
	} else if (RC_NEAR(delay, RC5_SHORT)) {
		// nothing to change
	} else {
		rc5Cnt = 0;									// Reset
		rc5State = STATE_RC5_MID1;
	}

	if (rc5Cnt == 0)
		rc5Cmd = 0x01;

	newstate = (pgm_read_byte(&trans[rc5State]) >> event) & 0x03;

	if (newstate == rc5State || rc5State > STATE_RC5_START0) {
		rc5Cnt = 0;									// Reset
	} else {
		rc5State = newstate;

		if (rc5State == STATE_RC5_MID0 || rc5State == STATE_RC5_MID1) {
			rc5Cnt++;
			rc5Cmd <<= 1;
		}

		if (rc5State == STATE_RC5_MID1)
			rc5Cmd |= 0x01;

		if (rc5Cnt >= 13 && (rc5State == STATE_RC5_START1 || rc5State == STATE_RC5_MID0)) {
			rc5TogBit = (rc5Cmd & RC5_TOGB_MASK) != 0;

			irData.ready = 1;
			irData.repeat = (rc5TogBit == rc5TogBitOld);
			irData.type = IR_TYPE_RC5;
			irData.address  = (rc5Cmd & RC5_ADDR_MASK) >> 6;
			irData.command = rc5Cmd & RC5_COMM_MASK;

			rc5TogBitOld = rc5TogBit;
			rc5Cnt = 0;
		}
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
