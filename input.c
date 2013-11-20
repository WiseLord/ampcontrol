#include <avr/io.h>
#include <avr/interrupt.h>

#include "input.h"

volatile uint8_t commBuf = 0;
volatile uint8_t rc5Buf = 0;

volatile uint8_t rc5cnt = 0;

const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};
volatile uint16_t rc5cmd;
uint8_t ccounter;

rc5State state = STATE_BEGIN;

void rc5Reset()
{
	ccounter = 14;
	rc5cmd = 0;
	state = STATE_BEGIN;
	rc5cnt = 0;
}

void rc5Init()
{
	MCUCR |= (1<<ISC10);	// Set INT1 to trigger on any edge
	BTN_DDR &= ~(1<<PD3);	// Set PD3 (INT1) to input

	TCCR1A = 0;				// Reset Timer1 counter
	TCCR1B = (1<<CS11);		// Set Timer1 prescaler to 8 (2MHz)

	rc5Reset();

	GICR |= (1<<INT1);		// Enable INT1 interrupt
}

ISR(INT1_vect)
{
	uint16_t delay = TCNT1;

	/* event: 0 / 2 - short space/pulse, 4 / 6 - long space/pulse */
	uint8_t event = (BTN_PIN & RC5_PIN) ? 2 : 0;

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX)
		event += 4;
	else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX)
		rc5Reset();

	if (state == STATE_BEGIN) {
		ccounter--;
		rc5cmd |= (1 << ccounter);
		state = STATE_MID1;
		TCNT1 = 0;
		return;
	}

	rc5State newstate = (trans[state] >> event) & 0x03;

	if (newstate == state || state > STATE_START0)
	{
		rc5Reset();
		return;
	}

	state = newstate;

	if (state == STATE_MID0)
		ccounter--;
	else if (state == STATE_MID1)
	{
		ccounter--;
		rc5cmd |= 1 << ccounter;
	}

	if(ccounter == 0 && (state == STATE_START1 || state == STATE_MID0))
	{
		if ((rc5cmd & RC5_ADDR_MASK) == RC5_ADDR)
		{
			switch (rc5cmd & RC5_COMM_MASK) {
			case RC5_MENU:
				rc5Buf = COMM_BTN_MENU;
				break;
			case RC5_VOL_UP:
				rc5Buf = COMM_ENC_UP;
				break;
			case RC5_VOL_DOWN:
				rc5Buf = COMM_ENC_DOWN;
				break;
			case RC5_TIME:
				rc5Buf = COMM_SHOW_TIME;
				break;
			default:
				rc5Buf = 0;
				break;
			}
		}
		rc5Reset();
	}

	TCNT1 = 0;
}

void btnInit(void)
{
	BTN_DDR = 0x00;
	BTN_PORT = 0xFF;
	TCCR2 = 0b110;			// Set timer prescaller to 256 (62.5 kHz)
	OCR2 = 125;				// Value to compare => 62.5k/125 => 500 polls/sec
	TCCR2 |= (1<<WGM21);	// Reset counter on match
	TCNT2 = 0;				// Reset timer value
	TIMSK |= (1<<OCIE2);	// Enable timer compare match interrupt
}

ISR (TIMER2_COMP_vect) {
	static uint8_t encPrev = 0;
	static int16_t btnCnt = 0;

	uint8_t btnPin = ~BTN_PIN;
	uint8_t encNow = btnPin & (ENC_A | ENC_B);
	uint8_t btnNow = btnPin & BTN_MASK;

	switch (encNow) {
	case ENC_AB:
		if (encPrev == ENC_B) commBuf = COMM_ENC_UP;
		if (encPrev == ENC_A) commBuf = COMM_ENC_DOWN;
		break;
/*	case ENC_A:
		if (encPrev == ENC_AB) commBuf = COMM_ENC_UP;
		if (encPrev == ENC_0) commBuf = COMM_ENC_DOWN;
		break;
	case ENC_B:
		if (encPrev == ENC_0) commBuf = COMM_ENC_UP;
		if (encPrev == ENC_AB) commBuf = COMM_ENC_DOWN;
		break;
*/	case ENC_0:
		if (encPrev == ENC_A) commBuf = COMM_ENC_UP;
		if (encPrev == ENC_B) commBuf = COMM_ENC_DOWN;
		break;
	default:
		break;
	}
	encPrev = encNow;

	static uint8_t cmdPrev = 0;
	uint8_t cmdNow = rc5Buf;

	rc5cnt++;
	if (rc5cnt > 60)
	{
		rc5cnt = 60;
		rc5Buf = 0;
	}

	switch (btnNow) {
	case BTN_MENU:
		cmdNow = COMM_BTN_MENU;
		break;
	case BTN_RIGHT:
		cmdNow = COMM_ENC_UP;
		break;
	case BTN_LEFT:
		cmdNow = COMM_ENC_DOWN;
		break;
	case BTN_DOWN:
		cmdNow = COMM_SHOW_TIME;
		break;
	default:
		break;
	}

	if (cmdNow && ((cmdPrev != cmdNow) | (btnCnt >= LONG_PRESS)))
		commBuf = cmdNow;

	if (btnCnt >= LONG_PRESS)
	{
		switch (cmdNow) {
		case COMM_ENC_UP:
		case COMM_ENC_DOWN:
			btnCnt = LONG_PRESS - REPEAT_TIME;
			break;
		default:
			btnCnt = 0;
			break;
		}
	}
	cmdPrev = cmdNow;

	if (cmdNow)
		btnCnt++;
	else
		btnCnt = 0;

	return;
};

uint8_t getCommand(void) {
	uint8_t ret = commBuf;
	commBuf = 0;
	return ret;
}
