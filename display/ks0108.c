#include "ks0108.h"

#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#ifdef KS0108_CS_INV
#define KS0108_SET_CS1(); \
	PORT(KS0108_CS1) &= ~KS0108_CS1_LINE; \
	PORT(KS0108_CS2) |= KS0108_CS2_LINE;
#define KS0108_SET_CS2(); \
	PORT(KS0108_CS2) &= ~KS0108_CS2_LINE; \
	PORT(KS0108_CS1) |= KS0108_CS1_LINE;
#else
#define KS0108_SET_CS1(); \
	PORT(KS0108_CS1) |= KS0108_CS1_LINE; \
	PORT(KS0108_CS2) &= ~KS0108_CS2_LINE;
#define KS0108_SET_CS2(x); \
	PORT(KS0108_CS2) |= KS0108_CS2_LINE; \
	PORT(KS0108_CS1) &= ~KS0108_CS1_LINE;
#endif

static uint8_t fb[KS0108_COLS * KS0108_CHIPS][KS0108_ROWS];
static uint8_t _br;

static uint8_t adcTimer = 0;

void setAdcTimer(uint8_t value)
{
	adcTimer = value;

	return;
}

void ks0108SetBrightness(uint8_t br)
{
	_br = br;

	return;
}

static void ks0108SetPort(uint8_t data)
{
	if (data & (1<<0)) PORT(KS0108_D0) |= KS0108_D0_LINE; else PORT(KS0108_D0) &= ~KS0108_D0_LINE;
	if (data & (1<<1)) PORT(KS0108_D1) |= KS0108_D1_LINE; else PORT(KS0108_D1) &= ~KS0108_D1_LINE;
	if (data & (1<<2)) PORT(KS0108_D2) |= KS0108_D2_LINE; else PORT(KS0108_D2) &= ~KS0108_D2_LINE;
	if (data & (1<<3)) PORT(KS0108_D3) |= KS0108_D3_LINE; else PORT(KS0108_D3) &= ~KS0108_D3_LINE;
	if (data & (1<<4)) PORT(KS0108_D4) |= KS0108_D4_LINE; else PORT(KS0108_D4) &= ~KS0108_D4_LINE;
	if (data & (1<<5)) PORT(KS0108_D5) |= KS0108_D5_LINE; else PORT(KS0108_D5) &= ~KS0108_D5_LINE;
	if (data & (1<<6)) PORT(KS0108_D6) |= KS0108_D6_LINE; else PORT(KS0108_D6) &= ~KS0108_D6_LINE;
	if (data & (1<<7)) PORT(KS0108_D7) |= KS0108_D7_LINE; else PORT(KS0108_D7) &= ~KS0108_D7_LINE;

	return;
}

static void ks0108SetDdr(uint8_t data)
{
	if (data & (1<<0)) DDR(KS0108_D0) |= KS0108_D0_LINE; else DDR(KS0108_D0) &= ~KS0108_D0_LINE;
	if (data & (1<<1)) DDR(KS0108_D1) |= KS0108_D1_LINE; else DDR(KS0108_D1) &= ~KS0108_D1_LINE;
	if (data & (1<<2)) DDR(KS0108_D2) |= KS0108_D2_LINE; else DDR(KS0108_D2) &= ~KS0108_D2_LINE;
	if (data & (1<<3)) DDR(KS0108_D3) |= KS0108_D3_LINE; else DDR(KS0108_D3) &= ~KS0108_D3_LINE;
	if (data & (1<<4)) DDR(KS0108_D4) |= KS0108_D4_LINE; else DDR(KS0108_D4) &= ~KS0108_D4_LINE;
	if (data & (1<<5)) DDR(KS0108_D5) |= KS0108_D5_LINE; else DDR(KS0108_D5) &= ~KS0108_D5_LINE;
	if (data & (1<<6)) DDR(KS0108_D6) |= KS0108_D6_LINE; else DDR(KS0108_D6) &= ~KS0108_D6_LINE;
	if (data & (1<<7)) DDR(KS0108_D7) |= KS0108_D7_LINE; else DDR(KS0108_D7) &= ~KS0108_D7_LINE;

	return;
}

static void ks0108Write(uint8_t type, uint8_t data)
{
	_delay_us(50);

	if (type == KS0108_DATA)
		PORT(KS0108_DI) |= KS0108_DI_LINE;
	else
		PORT(KS0108_DI) &= ~KS0108_DI_LINE;

	PORT(KS0108_D0) |= KS0108_D0_LINE;

	ks0108SetPort(data);

	PORT(KS0108_E) |= KS0108_E_LINE;
	asm("nop");
	PORT(KS0108_E) &= ~KS0108_E_LINE;

	return;
}

static void ks0108TimerInit(void)
{
	TIMSK |= (1<<TOIE0);							/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (0<<CS00);		/* Set timer prescaller to 8 (2MHz) */

	return;
}

static uint8_t adcResultReady;

uint8_t isAdcResultReady(void)
{
	uint8_t ret = adcResultReady;
	adcResultReady = 0;

	return ret;
}

ISR (TIMER0_OVF_vect)
{
	/* 2MHz / (255 - 155) = 20000Hz => 10kHz Fourier analysis */
	TCNT0 = 155;									/* 20000Hz / 8 / 2 / 66 = 18.9 FPS */

	if (adcTimer)
		ADCSRA |= 1<<ADSC;

	static uint8_t i;
	static uint8_t j;
	static uint8_t cs;

	static uint8_t br;

	if (j == 64) {									/* Phase 1 (Y) */
		if (++i >= 8) {
			i = 0;
			if (++cs >= KS0108_CHIPS)
				cs = 0;
			switch (cs) {
			case 1:
				KS0108_SET_CS2();
				break;
			default:
				KS0108_SET_CS1();
				break;
			}
		}
		PORT(KS0108_DI) &= ~KS0108_DI_LINE;				/* Go to command mode */
		ks0108SetPort(KS0108_SET_PAGE + i);
	} else if (j == 65) {							/* Phase 2 (X) */
		ks0108SetPort(KS0108_SET_ADDRESS);
	} else {										/* Phase 3 (32 bytes of data) */
		ks0108SetPort(fb[j + 64 * cs][i]);
	}

	PORT(KS0108_E) |= KS0108_E_LINE;						/* Strob */
	asm("nop");
	PORT(KS0108_E) &= ~KS0108_E_LINE;

	if (++j >= 66) {
		j = 0;
		PORT(KS0108_DI) |= KS0108_DI_LINE;				/* Go to data mode */
	}

	if (++br >= KS0108_MAX_BRIGTHNESS)				/* Loop brightness */
		br = KS0108_MIN_BRIGHTNESS;

	if (br == _br) {
		PORT(KS0108_BCKL) &= ~KS0108_BCKL_LINE;			/* Turn backlight off */
	} else if (br == 0)
		PORT(KS0108_BCKL) |= KS0108_BCKL_LINE;			/* Turn backlight on */

	return;
}

void ks0108Init(void)
{
	/* Set control and data lines as outputs */
	DDR(KS0108_DI) |= KS0108_DI_LINE;
	DDR(KS0108_RW) |= KS0108_RW_LINE;
	DDR(KS0108_E) |= KS0108_E_LINE;
	DDR(KS0108_CS1) |= KS0108_CS1_LINE;
	DDR(KS0108_CS2) |= KS0108_CS2_LINE;
	DDR(KS0108_RES) |= KS0108_RES_LINE;
	ks0108SetDdr(0xFF);

	PORT(KS0108_RW) &= ~(KS0108_RW_LINE);
	PORT(KS0108_DI) &= ~(KS0108_DI_LINE);
	PORT(KS0108_E) &= ~(KS0108_E_LINE);

	/* Reset */
	PORT(KS0108_RES) &= ~(KS0108_RES_LINE);
	asm("nop");
	asm("nop");
	PORT(KS0108_RES) |= KS0108_RES_LINE;
	asm("nop");
	asm("nop");

	/* Init first controller */
	KS0108_SET_CS1();
	ks0108Write(KS0108_COMMAND, KS0108_DISPLAY_START_LINE);
	ks0108Write(KS0108_COMMAND, KS0108_DISPLAY_ON);
	/* Init second controller */
	KS0108_SET_CS2();
	ks0108Write(KS0108_COMMAND, KS0108_DISPLAY_START_LINE);
	ks0108Write(KS0108_COMMAND, KS0108_DISPLAY_ON);

	PORT(KS0108_DI) |= KS0108_DI_LINE;

	ks0108TimerInit();

	DDR(KS0108_BCKL) |= KS0108_BCKL_LINE;

	return;
}

void ks0108Clear()
{
	uint8_t i, j;

	for (i = 0; i < KS0108_COLS * KS0108_CHIPS; i++) {
		for (j = 0; j < KS0108_ROWS; j++) {
			fb[i][j] = 0x00;
		}
	}

	return;
}

void ks0108DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
	uint8_t bit;

	if (x >= KS0108_COLS * KS0108_CHIPS)
		return;
	if (y >= KS0108_ROWS * 8)
		return;

	bit = 1 << (y & 0x07);

	if (color)
		fb[x][y >> 3] |= bit;
	else
		fb[x][y >> 3] &= ~bit;

	return;
}
