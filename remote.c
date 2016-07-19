#include "remote.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static volatile IRData irData;			// Last decoded IR command

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

	static uint8_t rc5Cnt;		// RC5 bit counter
	static uint16_t rc5Cmd;		// RC5 command
	static uint8_t rc5TogBitOld = 0;
	uint8_t rc5TogBit;

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX) {
		// Long period
		rc5Cnt--;
		if (pin)
			rc5Cmd &= ~(1 << rc5Cnt);
		else
			rc5Cmd |= (1 << rc5Cnt);
		TCNT1 = 0;
	} else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX) {
		// Reset state
		rc5Cnt = 13;
		rc5Cmd = (1 << rc5Cnt);		// Set bit 13 to 1
		TCNT1 = 0;
		return;
	}

	// If command decoded, place it to buffer
	if (rc5Cnt == 0) {
		rc5TogBit = (rc5Cmd & RC5_TOGB_MASK) != 0;

		irData.ready = 1;
		irData.repeat = (rc5TogBit == rc5TogBitOld);
//		irData.type = IR_TYPE_RC5;
		irData.address  = (rc5Cmd & RC5_ADDR_MASK) >> 6;
		irData.command = rc5Cmd & RC5_COMM_MASK;

		rc5TogBitOld = rc5TogBit;
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
