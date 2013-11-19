#include <avr/io.h>
#include <avr/interrupt.h>

#include "input.h"


const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};
volatile uint16_t rc5cmd;
uint8_t ccounter;
volatile uint8_t has_new;
State state = STATE_BEGIN;

void rc5Init()
{
	MCUCR |= (1<<ISC10); // Set INT1 to trigger on any edge
	BTN_DDR &= ~(1<<PD3); // Set PD3 (INT1) to input

	TCCR1A = 0; // Reset Timer1 counter
	TCCR1B = (1<<CS11); // Set Timer1 prescaler to 8 (2MHz)

	rc5Reset();
}

void rc5Reset()
{
	has_new = 0;
	ccounter = 14;
	rc5cmd = 0;
	state = STATE_BEGIN;

	GICR |= (1<<INT1); // Enable INT1 interrupt
}

uint8_t rc5NewComm(uint16_t *newCmd)
{
	if (has_new)
		*newCmd = rc5cmd;
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
	uint8_t event = (BTN_PIN & RC5_PIN) ? 2 : 0;

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX)
		event += 4;
	else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX)
		rc5Reset();

	if (state == STATE_BEGIN)
	{
		ccounter--;
		rc5cmd |= (1 << ccounter);
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

	/* Emit 0 - just decrement bit position counter
	 * cause data is already zeroed by default. */
	if (state == STATE_MID0)
		ccounter--;
	else if (state == STATE_MID1)
	{
		/* Emit 1 */
		ccounter--;
		rc5cmd |= 1 << ccounter;
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




uint8_t getRC5Comm(void)
{
	uint16_t rc5Comm;
	uint8_t ret = 0;
	if (rc5NewComm(&rc5Comm))
		rc5Reset();
	if ((rc5Comm & RC5_COMM_MASK) == RC5_VOL_UP)
		ret = COMM_ENC_UP;
	if ((rc5Comm & RC5_COMM_MASK) == RC5_VOL_DOWN)
		ret = COMM_ENC_DOWN;
	if ((rc5Comm & RC5_COMM_MASK) == RC5_MENU)
		ret = COMM_BTN_MENU;
	return ret;
}






void btnInit(void)
{
	BTN_DDR = 0x00;
	BTN_PORT = 0xFF;
	TCCR2 = 0b110; // Set timer prescaller to 256 (62.5 kHz)
	OCR2 = 125; // Value for compare => 62.5k/125 => 500 polls/sec
	TCCR2 |= (1<<WGM21); // Reset counter on match
	TCNT2 = 0; // Reset timer
	TIMSK |= (1<<OCIE2); // Enable timer compare match interrupt
}

volatile uint8_t commBuf = 0;

ISR (TIMER2_COMP_vect) {
	static uint8_t encPrev = 0;
	static uint8_t btnPrev = 0;
	static uint16_t btnCnt = 0;

	uint8_t btnPin = ~BTN_PIN;
	uint8_t encNow = btnPin & (ENC_A | ENC_B);
	uint8_t btnNow = btnPin & BTN_MASK;

	switch (encNow) {
	case ENC_AB:
		if (encPrev == ENC_B) commBuf = COMM_ENC_UP;
		if (encPrev == ENC_A) commBuf = COMM_ENC_DOWN;
		break;
	case ENC_A:
//		if (encPrev == ENC_AB) commBuf = COMM_ENC_UP;
//		if (encPrev == ENC_0) commBuf = COMM_ENC_DOWN;
		break;
	case ENC_B:
//		if (encPrev == ENC_0) commBuf = COMM_ENC_UP;
//		if (encPrev == ENC_AB) commBuf = COMM_ENC_DOWN;
		break;
	default:
		if (encPrev == ENC_A) commBuf = COMM_ENC_UP;
		if (encPrev == ENC_B) commBuf = COMM_ENC_DOWN;
		break;
	}
	encPrev = encNow;

	switch (btnNow) {
	case BTN_MENU:
		if ((btnPrev != BTN_MENU) | (btnCnt == 400))
		{
			commBuf = COMM_BTN_MENU;
			btnCnt = 0;
		}
		break;
	case BTN_RIGHT:
		if ((btnPrev != BTN_RIGHT) | (btnCnt == 400))
		{
			commBuf = COMM_ENC_UP;
			if (btnCnt == 400)
				btnCnt = 300;
		}
		break;
	case BTN_LEFT:
		if ((btnPrev != BTN_LEFT) | (btnCnt == 400))
		{
			commBuf = COMM_ENC_DOWN;
			if (btnCnt == 400)
				btnCnt = 300;
		}
		break;
	default:
		break;
	}
	btnPrev = (btnNow);

	if (btnNow == 0)
		btnCnt = 0;
	else
		btnCnt++;

	return;
};

uint8_t getBtnComm(void) {
	uint8_t ret = commBuf;
	commBuf = 0;
	return ret;
}
