#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"

void adcInit()
{
	ADCSRA |= (1<<ADEN);				// ADC enable
	ADCSRA |= (1<<ADPS2) | (1<<ADPS0);	// Set ADC prescaler to 32 (250kHz)
	ADCSRA |= (1<<ADIE);				// Enable ADC complete interrupt
	return;
}

void timerInit (void)
{
	TIMSK |= (1<<OCIE0);	// Enable timer compare match interrupt
	TCCR0 = 0b010;			// Set timer prescaller to 8 (2 MHz)
	OCR0 = 125;				// Value for compare => 2000000/125=16000 meas/sec
	TCCR0 |= (1<<WGM01);	// Reset counter on match
	return;
}

volatile uint8_t ind = 0;
int16_t f_r[FFT_SIZE];
int16_t f_i[FFT_SIZE];

ISR (TIMER0_COMP_vect) {
	ADCSRA |= 1<<ADSC;
	return;
};

ISR (ADC_vect)
{
	f_r[ind] = ADCL;
	f_r[ind] += (ADCH << 8);
	f_r[ind] -= 507; // Value from ADC when no signal
	f_i[ind] = 0;
	ind++;
	return;
}

ISR (INT0_vect)
{
	gdFill(0x00, CS1 | CS2);
	_delay_ms(500);
	return;
}

void getValues()
{
	ind = 0;
	sei();
	while (ind < FFT_SIZE);
	cli();
	return;
}

int16_t buf[FFT_SIZE / 2];

void slowFall()
{
	uint8_t i;
	const uint8_t fs = 2;
	for (i = 0; i < FFT_SIZE / 2; i++)
	{
		if (f_r[i] + fs <= buf[i])
			buf[i] -= fs;
		else
			buf[i] = f_r[i];
	}
	return;
}

void fft_loop()
{
	getValues();
	hammWindow(f_r);
	rev_bin(f_r);
	fft_radix4(f_r, f_i);
	cplx2dB(f_r, f_i);
	slowFall();
	gdGraph(buf);
	return;
}

int main(void)
{
	_delay_ms(100);
	gdInit();
	timerInit();
	adcInit();

	while (1)
	{
		fft_loop();
	}
	return 0;
}
