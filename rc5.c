#include "rc5.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define SHORT_MIN 888   /* 444 microseconds */
#define SHORT_MAX 2666  /* 1333 microseconds */
#define LONG_MIN 2668   /* 1334 microseconds */
#define LONG_MAX 4444   /* 2222 microseconds */

typedef enum {
	STATE_START1,
	STATE_MID1,
	STATE_MID0,
	STATE_START0,
	STATE_ERROR,
	STATE_BEGIN,
	STATE_END
} State;

const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};
volatile uint16_t command;
uint8_t ccounter;
volatile uint8_t has_new;
State state = STATE_BEGIN;

void rc5Init()
{
	MCUCR |= (1<<ISC10); // Set INT1 to trigger on any edge
	DDRD &= ~(1<<PD3); // Set PD3 (INT1) to input

	TCCR1A = 0; // Reset Timer1 counter
	TCCR1B = (1<<CS11); // Set Timer1 prescaler to 8 (2MHz)

	rc5Reset();
}

void rc5Reset()
{
	has_new = 0;
	ccounter = 14;
	command = 0;
	state = STATE_BEGIN;

	GICR |= (1<<INT1); // Enable INT1 interrupt
}

uint8_t rc5NewComm(uint16_t *new_command)
{
	if (has_new)
		*new_command = command;
	return has_new;
}

ISR(INT1_vect)
{
	uint16_t delay = TCNT1;

	/* TSOP2236 pulls the data line up, giving active low,
	 * so the output is inverted. If data pin is high then the edge
	 * was falling and vice versa.
	 *  Event numbers:
	 *  0 - short space
	 *  2 - short pulse
	 *  4 - long space
	 *  6 - long pulse
	 */
	uint8_t event = (PIND & (1<<PD3)) ? 2 : 0;

	if (delay > LONG_MIN && delay < LONG_MAX)
		event += 4;
	else if (delay < SHORT_MIN || delay > SHORT_MAX)
	{
		/* If delay wasn't long and isn't short then
		 * it is erroneous so we need to reset but
		 * we don't return from interrupt so we don't
		 * loose the edge currently detected. */
		rc5Reset();
	}

	if (state == STATE_BEGIN)
	{
		ccounter--;
		command |= (1 << ccounter);
		state = STATE_MID1;
		TCNT1 = 0;
		return;
	}

	State newstate = (trans[state] >> event) & 0x03;

	if (newstate == state || state > STATE_START0)
	{
		/* No state change or wrong state means
		 * error so reset. */
		rc5Reset();
		return;
	}

	state = newstate;

	/* Emit 0 - jest decrement bit position counter
	 * cause data is already zeroed by default. */
	if (state == STATE_MID0)
		ccounter--;
	else if (state == STATE_MID1)
	{
		/* Emit 1 */
		ccounter--;
		command |= 1 << ccounter;
	}

	/* The only valid end states are MID0 and START1.
	 * MID0 is ok, but if we finish in MID1 we need to wait
	 * for START1 so the last edge is consumed. */
	if(ccounter == 0 && (state == STATE_START1 || state == STATE_MID0))
	{
		state = STATE_END;
		has_new = 1;

		GICR &= ~(1<<INT1); // Disable INT1 interrupt
	}

	TCNT1 = 0;
}
