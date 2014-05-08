#include "rc5.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint8_t rc5Cnt;			/* RC5 bit counter */
static volatile uint16_t rc5Cmd;		/* RC5 command */
static volatile rc5State state;			/* Decoding process status */

static volatile uint16_t rc5RawBuf = 0;	/* Last decoded RC5 command */

static const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};

static void rc5Reset()
{
	rc5Cnt = 14;
	rc5Cmd = 0;
	state = STATE_BEGIN;

	return;
}

void rc5Init(void)
{
	MCUCR |= (1<<ISC10);				/* Set INT1 to trigger on any edge */
	RC5_DDR &= ~RC5_DATA;				/* Set PD3 (INT1) to input */
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
	uint8_t event = (RC5_PIN & RC5_DATA) ? 2 : 0;

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX)
		event += 4;
	else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX)
		rc5Reset();

	if (state == STATE_BEGIN) {
		rc5Cnt--;
		rc5Cmd |= (1 << rc5Cnt);
		state = STATE_MID1;
		TCNT1 = 0;

		return;
	}

	rc5State newstate = (trans[state] >> event) & 0x03;

	if (newstate == state || state > STATE_START0) {
		rc5Reset();

		return;
	}

	state = newstate;

	if (state == STATE_MID0)
		rc5Cnt--;
	else if (state == STATE_MID1) {
		rc5Cnt--;
		rc5Cmd |= 1 << rc5Cnt;
	}

	if (rc5Cnt == 0 && (state == STATE_START1 || state == STATE_MID0)) {
		rc5RawBuf = rc5Cmd;
		rc5Reset();
	}

	TCNT1 = 0;

	return;
}

uint16_t getRC5RawBuf(void)
{
	uint16_t ret = rc5RawBuf;
	rc5RawBuf = RC5_BUF_EMPTY;
	return ret;
}
