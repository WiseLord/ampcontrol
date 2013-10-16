#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"

volatile uint8_t ind = 0;
int16_t f_l[FFT_SIZE];		// Real values for left channel
int16_t f_r[FFT_SIZE];		// Real values for right channel
int16_t f_i[FFT_SIZE];		// Imaginary values
uint8_t buf[FFT_SIZE];		// Buffer with previous results

static inline void adcPerform()
{
	ADCSRA |= 1<<ADSC;							// Start measure
	while ((ADCSRA & (1<<ADSC)) == (1<<ADSC));	// Wait for finish
	return;
}

void adcInit()
{
	ADCSRA |= (1<<ADEN);				// ADC enable
	ADCSRA |= (1<<ADPS2) | (1<<ADPS0);	// Set ADC prescaler to 32 (500 kHz)
	adcPerform();
	return;
}

void timerInit (void)
{
	TCCR0 = 0b010;			// Set timer prescaller to 8 (2 MHz)
	OCR0 = 143;				// Value for compare => 2000000/143=14k meas/sec
	TCCR0 |= (1<<WGM01);	// Reset counter on match
	return;
}

ISR (TIMER0_COMP_vect) {
	ADMUX &= ~(1<<MUX0);			// Switch to left chnnel
	adcPerform();
	f_l[ind] = ADCL + (ADCH << 8);
	f_l[ind] -= 507;

	ADMUX |= (1<<MUX0);				// Switch to right channel
	adcPerform();
	f_r[ind] = ADCL + (ADCH << 8);
	f_r[ind] -= 507;

	f_i[ind++] = 0;
	return;
};


void getValues()
{
	ind = 0;
	TIMSK |= (1<<OCIE0);	// Enable timer compare match interrupt
	while (ind < FFT_SIZE);
	TIMSK &= ~(1<<OCIE0);
	return;
}

void slowFall(uint8_t fallRate)
{
	uint8_t i;
	for (i = 0; i < FFT_SIZE / 2; i++)
	{
		if (fallRate && (f_l[i] + fallRate <= buf[i]))
			buf[i] -= fallRate;
		else
			buf[i] = f_l[i];
		if (fallRate && (f_r[i] + fallRate <= buf[i + FFT_SIZE / 2]))
			buf[i + FFT_SIZE / 2] -= fallRate;
		else
			buf[i + FFT_SIZE / 2] = f_r[i];
	}
	return;
}

void fft_loop()
{
	getValues();

	hammWindow(f_l);
	revBin(f_l);
	fftRad4(f_l, f_i);
	cplx2dB(f_l, f_i);

	hammWindow(f_r);
	revBin(f_r);
	fftRad4(f_r, f_i);
	cplx2dB(f_r, f_i);

	slowFall(2);
	gdSpectrum(buf, MODE_STEREO);
	return;
}

int main(void)
{
	_delay_ms(100);
	gdInit();
	timerInit();
	adcInit();

	sei();

	while (1)
	{
		fft_loop();
	}
	return 0;
}
