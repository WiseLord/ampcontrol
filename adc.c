#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "adc.h"
#include "fft.h"

#include "display/st7920.h"

static int16_t f_l[FFT_SIZE];	/* Real values for left channel */
static int16_t f_r[FFT_SIZE];	/* Real values for right channel */
static int16_t f_i[FFT_SIZE];	/* Imaginary values */
static uint8_t buf[FFT_SIZE];	/* Previous fft results: both left and right */

static const uint8_t hannTable[] PROGMEM = {
	  0,   1,   3,   6,  10,  16,  22,  30,
	 38,  48,  58,  69,  81,  93, 105, 118,
	131, 143, 156, 168, 180, 191, 202, 212,
	221, 229, 236, 242, 247, 251, 254, 255,
};

void adcInit()
{
	/* Enable ADC with prescaler 32 */
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (0<<ADPS1) | (1<<ADPS0);

	ADMUX |= (1<<ADLAR);						/* Adjust result to left */

	return;
}

static uint8_t revBits(uint8_t x)
{
	x = ((x & 0x15) << 1) | ((x & 0x2A) >> 1);	/* 00abcdef => 00badcfe */
	x = (x & 0x0C) | swap(x & 0x33);			/* 00badcfe => 00fedcba */

	return x;
}

static void getValues()
{
	uint8_t i = 0, j;
	uint8_t hv;

	ADMUX &= ~(1<<MUX0);						/* Switch to left channel */
	isAdcResultReady();							/* Reset ready flag */
	while(!isAdcResultReady());					/* Wait for new result */

	do {
		j = revBits(i);
		if (i < FFT_SIZE / 2)
			hv = pgm_read_byte(&hannTable[i]);
		else
			hv = pgm_read_byte(&hannTable[FFT_SIZE - 1 - i]);

		f_l[j] = ADCH - DC_CORR;				/* Read left channel value */
		f_l[j] = ((int32_t)hv * f_l[j]) >> 6;	/* Apply Hann window */

		ADMUX |= (1<<MUX0);						/* Switch to right channel */

		while(!isAdcResultReady());				/* Wait for new result */

		f_r[j] = ADCH - DC_CORR;				/* Read right channel value */
		f_r[j] = ((int32_t)hv * f_r[j]) >> 6;	/* Apply Hann window */

		ADMUX &= ~(1<<MUX0);					/* Switch to left channel */

		while(!isAdcResultReady());				/* Wait for new result */

		f_i[i++] = 0;
	} while (i < FFT_SIZE);

	return;
}

/* NEW value is displayed if bigger then OLD. Otherwise OLD-1 is displayed */
static void slowFall()
{
	uint8_t i, j;
	for (i = 0, j = FFT_SIZE / 2; i < FFT_SIZE / 2; i++, j++) {
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

uint8_t *getSpData()
{
	getValues();

	fftRad4(f_l, f_i);
	cplx2dB(f_l, f_i);

	fftRad4(f_r, f_i);
	cplx2dB(f_r, f_i);

	slowFall();

	return buf;
}
