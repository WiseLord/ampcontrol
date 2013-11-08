#include <avr/io.h>
#include <avr/interrupt.h>

#include "input.h"

volatile int8_t cnt;

void pollInit(void)
{
	TCCR2 = 0b110; // Set timer prescaller to 256 (62.5 kHz)
//	OCR2 = 25; // Value for compare => 62.5k/25 => 2500 polls/sec
	OCR2 = 125; // Value for compare => 62.5k/125 => 500 polls/sec
	TCCR2 |= (1<<WGM21); // Reset counter on match
	TCNT2 = 0; // Reset timer
	TIMSK |= (1<<OCIE2); // Enable timer compare match interrupt
	return;
}

void btnInit(void)
{
	cnt = 0;
	BTN_DDR = 0x00;
	BTN_PORT = 0xFF;
	pollInit();
}

ISR (TIMER2_COMP_vect) {
	static uint8_t ePrev = 0, eNow = 0;

	eNow = ~PIND & (ENC_A | ENC_B);
	switch (eNow) {
	case ENC_AB:
		if (ePrev == ENC_B) cnt++;
		if (ePrev == ENC_A) cnt--;
		break;
	case ENC_A:
//		if (ePrev == ENC_AB) cnt++;
//		if (ePrev == ENC_0) cnt--;
		break;
	case ENC_B:
//		if (ePrev == ENC_0) cnt++;
//		if (ePrev == ENC_AB) cnt--;
		break;
	default:
		if (ePrev == ENC_A) cnt++;
		if (ePrev == ENC_B) cnt--;
		break;
	}
	ePrev = eNow;
	return;
};

int8_t getEncValue(void)
{
	int8_t ret = cnt;
	cnt = 0;
	return ret;
}

uint8_t getButtons(void) {
	return (~BTN_PIN & BTN_MASK);
}
