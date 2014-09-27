#include "ks0108.h"

#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#ifdef KS0108_CS_INV
#define KS0108_SET_CS(x); KS0108_CTRL_PORT |= KS0108_CS_ALL; KS0108_CTRL_PORT &= ~(x);
#else
#define KS0108_SET_CS(x); KS0108_CTRL_PORT &= ~KS0108_CS_ALL; KS0108_CTRL_PORT |= (x);
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

static void ks0108Write(uint8_t type, uint8_t data)
{
	_delay_us(50);

	if (type == KS0108_DATA)
		KS0108_CTRL_PORT |= KS0108_DI;
	else
		KS0108_CTRL_PORT &= ~KS0108_DI;

	KS0108_DATA_PORT = data;

	KS0108_CTRL_PORT |= KS0108_E;
	asm("nop");
	KS0108_CTRL_PORT &= ~KS0108_E;

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
	if (adcTimer)
		ADCSRA |= 1<<ADSC;

	/* 2MHz / (255 - 155) = 20000Hz => 10kHz Fourier analysis */
	TCNT0 = 155;									/* 20000Hz / 8 / 2 / 66 = 18.9 FPS */

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
				KS0108_SET_CS(KS0108_CS2);
				break;
			default:
				KS0108_SET_CS(KS0108_CS1);
				break;
			}
		}
		KS0108_CTRL_PORT &= ~KS0108_DI;				/* Go to command mode */
		KS0108_DATA_PORT = KS0108_SET_PAGE + i;		/* Set Y */
	} else if (j == 65) {							/* Phase 2 (X) */
		KS0108_DATA_PORT = KS0108_SET_ADDRESS;		/* Set X */
	} else {										/* Phase 3 (32 bytes of data) */
		KS0108_DATA_PORT = fb[j + 64 * cs][i];
	}

	KS0108_CTRL_PORT |= KS0108_E;					/* Strob */
	asm("nop");
	KS0108_CTRL_PORT &= ~KS0108_E;

	if (++j >= 66) {
		j = 0;
		KS0108_CTRL_PORT |= KS0108_DI;				/* Go to data mode */
	}

	if (++br >= KS0108_MAX_BRIGTHNESS)				/* Loop brightness */
		br = KS0108_MIN_BRIGHTNESS;

	if (br == _br) {
		KS0108_BCKL_PORT &= ~KS0108_BCKL;			/* Turn backlight off */
	} else if (br == 0)
		KS0108_BCKL_PORT |= KS0108_BCKL;			/* Turn backlight on */

	return;
}

void ks0108Init(void)
{
	/* Set control and data lines as outputs */
	KS0108_CTRL_DDR |= KS0108_DI | KS0108_RW | KS0108_E;
	KS0108_CTRL_DDR |= KS0108_CS_ALL | KS0108_RES;
	KS0108_DATA_DDR = 0xFF;

	/* Reset */
	KS0108_CTRL_PORT &= ~(KS0108_RES);
	asm("nop");
	asm("nop");
	KS0108_CTRL_PORT |= KS0108_RES;
	asm("nop");
	asm("nop");

	KS0108_SET_CS(KS0108_CS_ALL);
	KS0108_CTRL_PORT &= ~KS0108_RW;

	ks0108Write(KS0108_COMMAND, KS0108_DISPLAY_START_LINE);
	ks0108Write(KS0108_COMMAND, KS0108_DISPLAY_ON);

	KS0108_CTRL_PORT |= KS0108_DI;

	ks0108TimerInit();

	KS0108_BCKL_DDR |= KS0108_BCKL;

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
