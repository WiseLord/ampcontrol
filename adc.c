#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "adc.h"
#include "fft.h"

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

static uint8_t _br;

void adcInit()
{
	TIMSK |= (1<<TOIE0);						/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (0<<CS00);	/* Set timer prescaller to 8 (2MHz) */

	/* Enable ADC with prescaler 16 */
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);

	ADMUX |= (1<<ADLAR);						/* Adjust result to left */

	return;
}

ISR (TIMER0_OVF_vect)
{
	/* 2MHz / (255 - 155) = 20000Hz => 10kHz Fourier analysis */
	TCNT0 = 155;								/* 20000 / 32 / 34 = 18.4 FPS */

	ADCSRA |= 1<<ADSC;

	static uint8_t br;

	if (++br >= DISP_MAX_BR)					/* Loop brightness */
		br = DISP_MIN_BR;

	if (br == _br) {
		BCKL_PORT &= ~BCKL;						/* Turn backlight off */
	} else if (br == 0)
		BCKL_PORT |=BCKL;						/* Turn backlight on */

	return;
};

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
	while (!(ADCSRA & (1<<ADSC)));				/* Wait for start measure */

	do {
		j = revBits(i);
		if (i < FFT_SIZE / 2)
			hv = pgm_read_byte(&hannTable[i]);
		else
			hv = pgm_read_byte(&hannTable[FFT_SIZE - 1 - i]);

		while (ADCSRA & (1<<ADSC));				/* Wait for finish measure */
		ADMUX |= (1<<MUX0);						/* Switch to right channel */
		f_l[j] = ADCH - DC_CORR;				/* Read left channel value */
		f_l[j] = ((int32_t)hv * f_l[j]) >> 6;	/* Apply Hann window */
		while (!(ADCSRA & (1<<ADSC)));			/* Wait for start measure */

		while (ADCSRA & (1<<ADSC));				/* Wait for finish measure */
		ADMUX &= ~(1<<MUX0);					/* Switch to left channel */
		f_r[j] = ADCH - DC_CORR;				/* Read right channel value */
		f_r[j] = ((int32_t)hv * f_r[j]) >> 6;	/* Apply Hann window */
		while (!(ADCSRA & (1<<ADSC)));			/* Wait for start measure */

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

void setDispBr(uint8_t br)
{
	_br = br;

	return;
}
