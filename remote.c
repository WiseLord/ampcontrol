#include "remote.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint8_t rc5Cnt;			// RC5 bit counter
static volatile uint16_t rcCmd;		// RC5 command

static volatile uint16_t rc5RawBuf = 0;	// Last decoded RC5 command

void rcInit(void)
{
	MCUCR |= (1<<ISC10);				// Set INT1 to trigger on any edge
	DDR(RC) &= ~RC_LINE;				// Set PD3 (INT1) to input
	TCCR1A = 0;							// Reset Timer1 counter
	TCCR1B = (1<<CS11);					// Set Timer1 prescaler to 8 (1MHz)
	GICR |= (1<<INT1);					// Enable INT1 interrupt

	return;
}

ISR(INT1_vect)
{
	uint16_t delay = TCNT1;

	uint8_t pin = PIN(RC) & RC_LINE;

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX) {
		// Long period
		rc5Cnt--;
		if (pin)
			rcCmd &= ~(1 << rc5Cnt);
		else
			rcCmd |= (1 << rc5Cnt);
		TCNT1 = 0;
	} else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX) {
		// Reset state
		rc5Cnt = 13;
		rcCmd = (1 << rc5Cnt);		// Set bit 13 to 1
		TCNT1 = 0;
		return;
	}

	// If command decoded, place it to buffer
	if (rc5Cnt == 0)
		rc5RawBuf = rcCmd;

	return;
}

uint16_t getRCRawBuf(void)
{
	uint16_t ret = rc5RawBuf;
	rc5RawBuf = RC5_BUF_EMPTY;
	return ret;
}
