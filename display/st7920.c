#include "st7920.h"

#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static uint8_t fb[ST7920_SIZE_X / 4][ST7920_SIZE_Y / 2];
static uint8_t _br;

static uint8_t adcTimer = 0;

void setAdcTimer(uint8_t value)
{
	adcTimer = value;

	return;
}

void st7920SetBrightness(uint8_t br)
{
	_br = br;

	return;
}

static void st7920Write(uint8_t type, uint8_t data)
{
	_delay_us(50);

	if (type == ST7920_DATA)
		ST7920_CTRL_PORT |= ST7920_RS;
	else
		ST7920_CTRL_PORT &= ~ST7920_RS;

	ST7920_DATA_PORT = data;

	ST7920_CTRL_PORT |= ST7920_E;
	asm("nop");
	ST7920_CTRL_PORT &= ~ST7920_E;

	return;
}

static void st7920TimerInit(void)
{
	TIMSK |= (1<<TOIE0);								/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (0<<CS00);			/* Set timer prescaller to 8 (2MHz) */

	return;
}

ISR (TIMER0_OVF_vect)
{
	/* 2MHz / (255 - 155) = 20000Hz => 10kHz Fourier analysis */
	TCNT0 = 155;										/* 20000 / 32 / 34 = 18.4 FPS */

	if (adcTimer)
		ADCSRA |= 1<<ADSC;

	static uint8_t i = 0;
	static uint8_t j = 32;

	static uint8_t br;

	if (j == 32) {										/* Phase 1 (Y) */
		ST7920_CTRL_PORT &= ~ST7920_RS;					/* Go to command mode */
		if (++i >= 32)
			i = 0;
		ST7920_DATA_PORT = ST7920_SET_GRAPHIC_RAM | i;	/* Set Y */
	} else if (j == 33) {								/* Phase 2 (X) */
		ST7920_DATA_PORT = ST7920_SET_GRAPHIC_RAM;		/* Set X */
	} else {											/* Phase 3 (32 bytes of data) */
		ST7920_DATA_PORT = fb[j][i];
	}

	ST7920_CTRL_PORT |= ST7920_E;						/* Strob */
	asm("nop");
	ST7920_CTRL_PORT &= ~ST7920_E;

	if (++j >= 34) {
		j = 0;
		ST7920_CTRL_PORT |= ST7920_RS;					/* Go to data mode */
	}

	if (++br >= ST7920_MAX_BRIGTHNESS)					/* Loop brightness */
		br = ST7920_MIN_BRIGHTNESS;

	if (br == _br) {
		ST7920_BCKL_PORT &= ~ST7920_BCKL;				/* Turn backlight off */
	} else if (br == 0)
		ST7920_BCKL_PORT |= ST7920_BCKL;				/* Turn backlight on */

	return;
}

void st7920Init(void)
{
	/* Set control and data lines as outputs */
	ST7920_CTRL_DDR |= (ST7920_RS | ST7920_RW | ST7920_E | ST7920_PSB | ST7920_RST);
	ST7920_DATA_DDR = 0xFF;

	ST7920_CTRL_PORT &= ~(ST7920_RS | ST7920_RW | ST7920_E);
	/* Switch display to parallel mode */
	ST7920_CTRL_PORT |= ST7920_PSB;

	/* Reset display */
	ST7920_CTRL_PORT &= ~ST7920_RST;
	_delay_us(1);
	ST7920_CTRL_PORT |= ST7920_RST;

	/* Init display in graphics mode */
	_delay_ms(40);
	st7920Write(ST7920_COMMAND, ST7920_FUNCTION | ST7920_8BIT);
	st7920Write(ST7920_COMMAND, ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR);
	st7920Write(ST7920_COMMAND, ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR | ST7920_GRAPHIC);

	st7920TimerInit();

	ST7920_BCKL_DDR |= ST7920_BCKL;

	return;
}

void st7920Clear()
{
	uint8_t i, j;

	for (i = 0; i < ST7920_SIZE_X / 4; i++) {
		for (j = 0; j < ST7920_SIZE_Y / 2; j++) {
			fb[i][j] = 0x00;
		}
	}

	return;
}

void st7920DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
	uint8_t bit;

	if (x >= ST7920_SIZE_X)
		return;
	if (y >= ST7920_SIZE_Y)
		return;

	bit = 0x80 >> (x & 0x07);

	if (y >= 32)
		x += 128;

	if (color)
		fb[x >> 3][y & 0x1F] |= bit;
	else
		fb[x >> 3][y & 0x1F] &= ~bit;

	return;
}
