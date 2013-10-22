#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "adc.h"
#include "fft.h"

int16_t f_l[FFT_SIZE]; // Real values for left channel
int16_t f_r[FFT_SIZE]; // Real values for right channel
int16_t f_i[FFT_SIZE]; // Imaginary values
uint8_t buf[FFT_SIZE]; // Buffer with previous results

void adcInit()
{
	TCCR0 = 0b010; // Set timer prescaller to 8 (2 MHz)
	OCR0 = 62; // Value for compare => 2000000/62 => 32k meas/sec
	TCCR0 |= (1<<WGM01); // Reset counter on match

	ADCSRA |= (1<<ADEN);
	ADCSRA |= (1<<ADPS2) | (1<<ADPS0); // ADC prescaler=32 (500 kHz)
	return;
}

ISR (TIMER0_COMP_vect) {
	ADCSRA |= 1<<ADSC; // Start measure
	return;
};

void getValues()
{
	uint8_t i = 0;
	TCNT0 = 0; // Reset timer
	TIMSK |= (1<<OCIE0); // Enable timer compare match interrupt

	ADMUX &= ~(1<<MUX0); // Switch ADC to left channel
	while ((ADCSRA & (1<<ADSC)) == 0); // Wait for start first measure

	do
	{
		while ((ADCSRA & (1<<ADSC)) == (1<<ADSC)); // Wait for finish measure
		ADMUX |= (1<<MUX0); // Switch to right channel
		f_l[i] = ADCL;
		f_l[i] += (ADCH << 8); // Read left channel value
		f_l[i] -= CORR_L;
		_delay_us(5);

		while ((ADCSRA & (1<<ADSC)) == (1<<ADSC)); // Wait for finish measure
		ADMUX &= ~(1<<MUX0); // Switch to left channel
		f_r[i] = ADCL;
		f_r[i] += (ADCH << 8); // Read right channel value
		f_r[i] -= CORR_R;
		_delay_us(5);

		f_i[i++] = 0;
	}
	while (i < FFT_SIZE);

	TIMSK &= ~(1<<OCIE0); // DIsable timer compare match interrupt
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

uint8_t *getData()
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
	return buf;
}
