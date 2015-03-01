#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "adc.h"
#include "fft.h"
#include "eeprom.h"

static int16_t fr[FFT_SIZE];							/* Real values */
static int16_t fi[FFT_SIZE];							/* Imaginary values */
static uint8_t buf[FFT_SIZE];							/* Previous results: left and right */

static uint8_t adcCorrLeft = DC_CORR;					/* Correction for left channel */
static uint8_t adcCorrRight = DC_CORR;					/* Correction for right channel */

static const uint8_t hannTable[] PROGMEM = {
	  0,   1,   3,   6,  10,  16,  22,  30,
	 38,  48,  58,  69,  81,  93, 105, 118,
	131, 143, 156, 168, 180, 191, 202, 212,
	221, 229, 236, 242, 247, 251, 254, 255,
};

static const int16_t dbTable[N_DB - 1] PROGMEM = {
	   1,    1,    2,    2,    3,    4,    6,    8,
	  10,   14,   18,   24,   33,   44,   59,   78,
	 105,  140,  187,  250,  335,  448,  599,  801,
	1071, 1432, 1915, 2561, 3425, 4580, 6125
};

void adcInit(void)
{
	/* Enable ADC with prescaler 16 */
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
	ADMUX |= (1<<ADLAR);								/* Adjust result to left (8bit ADC) */

	TIMSK |= (1<<TOIE0);								/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (0<<CS00);			/* Set timer prescaller to 8 (2MHz) */

	adcCorrLeft = eeprom_read_byte(eepromAdcCorrL);
	adcCorrRight = eeprom_read_byte(eepromAdcCorrR);

	return;
}

static uint8_t revBits(uint8_t x)
{
	x = ((x & 0x15) << 1) | ((x & 0x2A) >> 1);			/* 00abcdef => 00badcfe */
	x = (x & 0x0C) | swap(x & 0x33);					/* 00badcfe => 00fedcba */

	return x;
}

static void getValues(uint8_t mux)
{
	uint8_t i = 0, j;
	uint8_t hv;
	uint8_t dcCorr = DC_CORR;

	ADMUX &= ~((1<<MUX2) | (1<<MUX1) |(1<<MUX0));
	ADMUX |= mux;										/* Mux ADC to required audio channel */

	switch (mux) {										/* Set channel correction */
	case MUX_LEFT:
		dcCorr = adcCorrLeft;
		break;
	case MUX_RIGHT:
		dcCorr = adcCorrRight;
		break;
	}

	do {
		while (!(ADCSRA & (1<<ADSC)));					/* Wait for start measure */
		j = revBits(i);
		if (i < FFT_SIZE / 2)
			hv = pgm_read_byte(&hannTable[i]);
		else
			hv = pgm_read_byte(&hannTable[FFT_SIZE - 1 - i]);

		while (ADCSRA & (1<<ADSC));						/* Wait for finish measure */
		fr[j] = ADCH - dcCorr;							/* Read channel value */
		fr[j] = ((int32_t)hv * fr[j]) >> 6;				/* Apply Hann window */

		fi[i] = 0;
	} while (++i < FFT_SIZE);

	return;
}

static void cplx2dB(int16_t *fr, int16_t *fi)
{
	uint8_t i, j;
	int16_t calc;
	for (i = 0; i < FFT_SIZE / 2; i++) {
		calc = ((int32_t)fr[i] * fr[i] + (int32_t)fi[i] * fi[i]) >> 13;

		for (j = 0; j < N_DB - 1; j++)
			if (calc <= pgm_read_word(&dbTable[j]))
				break;
		fr[i] = j;
	}
	for (i = 0; i < FFT_SIZE; i++)
		fi[i] = 0;
	return;
}

uint8_t *getSpData(uint8_t fallSpeed)
{
	uint8_t i;

	getValues(MUX_LEFT);
	fftRad4(fr, fi);
	cplx2dB(fr, fi);

	for (i = 0; i < FFT_SIZE / 2; i++) {
		(buf[i] > fallSpeed) ? (buf[i] -= fallSpeed) : (buf[i] = 1);
		if (buf[i]-- <= fr[i])
			buf[i] = fr[i];
	}

	getValues(MUX_RIGHT);
	fftRad4(fr, fi);
	cplx2dB(fr, fi);

	for (i = FFT_SIZE / 2; i < FFT_SIZE; i++) {
		(buf[i] > fallSpeed) ? (buf[i] -= fallSpeed) : (buf[i] = 1);
		if (buf[i]-- <= fr[i - FFT_SIZE / 2])
			buf[i] = fr[i - FFT_SIZE / 2];
	}

	return buf;
}

uint16_t getSignalLevel(void)
{
	uint16_t ret = 0;
	uint8_t i;

	for (i = 0; i < sizeof(buf); i++)
		ret += buf[i];

	return ret * 3 / 64;
}
