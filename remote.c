#include "remote.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint8_t rc5Cnt;			// RC5 bit counter
static volatile uint16_t rc5Cmd;		// RC5 command
static volatile RC5State rc5State;		// Decoding process status

static volatile uint8_t necCnt;			// NEC bit counter
static volatile uint32_t necCmd;		// NEC command

static volatile IRData irData;			// Last decoded IR command
static volatile uint8_t ovfCnt = 250;	// Overflow counter

void rcInit(void)
{
	MCUCR |= (1<<ISC10);				/* Set INT1 to trigger on any edge */
	DDR(RC) &= ~RC_LINE;				// Set PD3 (INT1) to input
	TCCR1A = 0;							// Reset Timer1 counter
	TCCR1B = (1<<CS11);					// Set Timer1 prescaler to 8 (2MHz)
	GICR |= (1<<INT1);					/* Enable INT1 interrupt */

	TIMSK = (1<<TOIE1);					// Enable Timer1 overflow interrupt

	rc5State = STATE_RC5_BEGIN;
	necCnt = 32;

	return;
}

ISR(TIMER1_OVF_vect)					// Overflow every 33ms
{
	if (ovfCnt <= 250)
		ovfCnt++;

	return;
}

ISR(INT1_vect)
{
	uint8_t rcPin = !(PIN(RC) & RC_LINE);

	static uint16_t delay;
	static uint16_t tcnt;

	delay = TCNT1 - tcnt;
	tcnt = TCNT1;

	// Try to decode as NEC sequence
	if (rcPin) {
		necCnt--;
		if ((delay > NEC_ZERO_WIDTH_MIN) && (delay < NEC_ZERO_WIDTH_MAX)) {
			necCmd &= ~(1UL<<necCnt);
		} else if ((delay > NEC_ONE_WIDTH_MIN) && (delay < NEC_ONE_WIDTH_MAX)) {
			necCmd |= (1UL<<necCnt);
		} else if ((delay > NEC_REPEAT_WIDTH_MIN) && (delay < NEC_REPEAT_WIDTH_MAX)) {
			if (ovfCnt < 6) {				// Less then 200ms
				irData.ready = 1;
				irData.repeat = 1;
				ovfCnt = 0;
			}
		} else {
			necCnt = 32;				// Reset
		}
		if (necCnt == 0) {
			irData.ready = 1;
			irData.repeat = 0;
			irData.type = IR_TYPE_NEC;
			irData.address = (necCmd >> 24) & 0xFF;
			irData.command = (necCmd >> 8) & 0xFF;
			necCnt = 32;				// Reset
			ovfCnt = 0;
		}
	}

	// Try to decode as RC5 sequence
	uint8_t event = rcPin ? 0 : 2;	// 0/2 - short space/pulse, 4/6 - long space/pulse
	static const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX) {
		event += 4;
	} else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX) {
		rc5State = STATE_RC5_BEGIN;
	}

	if (rc5State == STATE_RC5_BEGIN) {
		rc5Cnt = 13;
		rc5Cmd |= (1 << rc5Cnt);
		rc5State = STATE_RC5_MID1;

		return;
	}

	RC5State newstate = (trans[rc5State] >> event) & 0x03;

	if (newstate == rc5State || rc5State > STATE_RC5_START0) {
		rc5State = STATE_RC5_BEGIN;

		return;
	}

	rc5State = newstate;

	if (rc5State == STATE_RC5_MID0) {
		rc5Cnt--;
		rc5Cmd &= ~(1 << rc5Cnt);
	} else if (rc5State == STATE_RC5_MID1) {
		rc5Cnt--;
		rc5Cmd |= (1 << rc5Cnt);
	}

	static uint8_t rc5TogBit = 0;
	if (rc5Cnt == 0 && (rc5State == STATE_RC5_START1 || rc5State == STATE_RC5_MID0)) {
		irData.ready = 1;
		if (!(rc5Cmd & RC5_TOGB_MASK) == rc5TogBit) {
			irData.repeat = 0;
			rc5TogBit = !rc5TogBit;
		} else {
			irData.repeat = 1;
		}
		irData.type = IR_TYPE_RC5;
		irData.address  = (rc5Cmd & RC5_ADDR_MASK) >> 6;
		irData.command = rc5Cmd & RC5_COMM_MASK;

		rc5State = STATE_RC5_BEGIN;
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
