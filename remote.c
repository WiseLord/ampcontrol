#include "remote.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint8_t rc5Cnt;			/* RC5 bit counter */
static volatile uint16_t rc5Cmd;		/* RC5 command */
static volatile RC5State rc5State;		/* Decoding process status */

static volatile IRData irData;			/* Last decoded IR command */

static const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};

static void rc5Reset()
{
	rc5Cnt = 14;
	rc5Cmd = 0;
	rc5State = STATE_RC5_BEGIN;

	return;
}

void rcInit(void)
{
	MCUCR |= (1<<ISC10);				/* Set INT1 to trigger on any edge */
	DDR(RC) &= ~RC_LINE;				/* Set PD3 (INT1) to input */
	TCCR1A = 0;							/* Reset Timer1 counter */
	TCCR1B = (1<<CS11);					/* Set Timer1 prescaler to 8 (2MHz) */
	GICR |= (1<<INT1);					/* Enable INT1 interrupt */

	rc5Reset();

	return;
}

ISR(INT1_vect)
{
	uint16_t delay = TCNT1;

	/* event: 0 / 2 - short space/pulse, 4 / 6 - long space/pulse */
	uint8_t event = (PIN(RC) & RC_LINE) ? 2 : 0;

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX)
		event += 4;
	else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX)
		rc5Reset();

	if (rc5State == STATE_RC5_BEGIN) {
		rc5Cnt--;
		rc5Cmd |= (1 << rc5Cnt);
		rc5State = STATE_RC5_MID1;
		TCNT1 = 0;

		return;
	}

	RC5State newstate = (trans[rc5State] >> event) & 0x03;

	if (newstate == rc5State || rc5State > STATE_RC5_START0) {
		rc5Reset();

		return;
	}

	rc5State = newstate;

	if (rc5State == STATE_RC5_MID0)
		rc5Cnt--;
	else if (rc5State == STATE_RC5_MID1) {
		rc5Cnt--;
		rc5Cmd |= 1 << rc5Cnt;
	}

	static uint8_t rc5TogBit = 0;
	if (rc5Cnt == 0 && (rc5State == STATE_RC5_START1 || rc5State == STATE_RC5_MID0)) {
		if (!(rc5Cmd & RC5_TOGB_MASK) == rc5TogBit) {
			irData.repeat = 0;
			rc5TogBit = !rc5TogBit;
		} else {
			irData.repeat = 1;
		}

		irData.type = IR_TYPE_RC5;
		irData.address  = (rc5Cmd & RC5_ADDR_MASK) >> 6;
		irData.command = rc5Cmd & RC5_COMM_MASK;
		rc5Reset();
	}

	TCNT1 = 0;

	return;
}

IRData takeIRData()
{
	IRData ret = irData;
	irData.type = IR_TYPE_NONE;

	return ret;
}

IRData getIrData()
{
	return irData;
}

void setIrData(uint8_t addr, uint8_t cmd)
{
	irData.address = addr;
	irData.command = cmd;

	return;
}
