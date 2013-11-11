#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "adc.h"
#include "fft.h"

#define swap(x) (__builtin_avr_swap(x)) // Swaps nibbles in byte

int16_t f_l[FFT_SIZE]; // Real values for left channel
int16_t f_r[FFT_SIZE]; // Real values for right channel
int16_t f_i[FFT_SIZE]; // Imaginary values
uint8_t buf[FFT_SIZE]; // Buffer with previous results

static const int16_t hammTable[] PROGMEM =
{
	 1257,  1295,  1407,  1593,  1851,  2178,  2571,  3027,
	 3539,  4105,  4717,  5370,  6057,  6772,  7507,  8255,
	 9009,  9761, 10504, 11229, 11931, 12602, 13236, 13825,
	14365, 14850, 15274, 15635, 15928, 16150, 16300, 16375,
};

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

static uint8_t revBits(uint8_t x)
{
	x = ((x & 0x15) << 1) | ((x & 0x2A) >> 1); // 00abcdef => 00badcfe
	x = (x & 0x0C) | swap(x & 0x33); // 00badcfe => 00fedcba
	return x;
}

void getValues()
{
	uint8_t i = 0, j;
	int32_t hv;
	TCNT0 = 0; // Reset timer
	TIMSK |= (1<<OCIE0); // Enable timer compare match interrupt

	ADMUX &= ~(1<<MUX0); // Switch ADC to left channel
	while ((ADCSRA & (1<<ADSC)) == 0); // Wait for start first measure

	do
	{
		j = revBits(i);
		if (i < FFT_SIZE / 2)
			hv = pgm_read_word(&hammTable[i]);
		else
			hv = pgm_read_word(&hammTable[FFT_SIZE - 1 - i]);

		while ((ADCSRA & (1<<ADSC)) == (1<<ADSC)); // Wait for finish measure
		ADMUX |= (1<<MUX0); // Switch to right channel
		f_l[j] = ADCL;
		f_l[j] += (ADCH << 8); // Read left channel value
		f_l[j] -= CORR_L;
		f_l[j] = (hv * f_l[j]) >> 14; // Apply Hamming window
		_delay_us(3); // Wait to be sure for new measure started

		while ((ADCSRA & (1<<ADSC)) == (1<<ADSC)); // Wait for finish measure
		ADMUX &= ~(1<<MUX0); // Switch to left channel
		f_r[j] = ADCL;
		f_r[j] += (ADCH << 8); // Read right channel value
		f_r[j] -= CORR_R;
		f_r[j] = (hv * f_r[j]) >> 14; // Apply Hamming window
		_delay_us(3); // Wait to be sure for new measure started

		f_i[i++] = 0;
	}
	while (i < FFT_SIZE);

	TIMSK &= ~(1<<OCIE0); // DIsable timer compare match interrupt
	return;
}

static void slowFall()
{
	uint8_t i, j;
	for (i = 0, j = FFT_SIZE / 2; i < FFT_SIZE / 2; i++, j++)
	{
		if (f_l[i] < buf[i])
			buf[i]--;
		else
			buf[i] = f_l[i];
		if (f_r[i] < buf[j])
			buf[j]--;
		else
			buf[j] = f_r[i];
	}
	return;
}

uint8_t *getData()
{
	getValues();

	fftRad4(f_l, f_i);
	cplx2dB(f_l, f_i);

	fftRad4(f_r, f_i);
	cplx2dB(f_r, f_i);

	slowFall();
	return buf;
}
