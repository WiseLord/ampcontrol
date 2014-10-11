#include "st7920.h"

#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static uint8_t fb[ST7920_SIZE_X / 4][ST7920_SIZE_Y / 2];
static uint8_t _br;

void st7920SetBrightness(uint8_t br)
{
	_br = br;

	return;
}

static void st7920SetPort(uint8_t data)
{
	if (data & (1<<0)) PORT(ST7920_D0) |= ST7920_D0_LINE; else PORT(ST7920_D0) &= ~ST7920_D0_LINE;
	if (data & (1<<1)) PORT(ST7920_D1) |= ST7920_D1_LINE; else PORT(ST7920_D1) &= ~ST7920_D1_LINE;
	if (data & (1<<2)) PORT(ST7920_D2) |= ST7920_D2_LINE; else PORT(ST7920_D2) &= ~ST7920_D2_LINE;
	if (data & (1<<3)) PORT(ST7920_D3) |= ST7920_D3_LINE; else PORT(ST7920_D3) &= ~ST7920_D3_LINE;
	if (data & (1<<4)) PORT(ST7920_D4) |= ST7920_D4_LINE; else PORT(ST7920_D4) &= ~ST7920_D4_LINE;
	if (data & (1<<5)) PORT(ST7920_D5) |= ST7920_D5_LINE; else PORT(ST7920_D5) &= ~ST7920_D5_LINE;
	if (data & (1<<6)) PORT(ST7920_D6) |= ST7920_D6_LINE; else PORT(ST7920_D6) &= ~ST7920_D6_LINE;
	if (data & (1<<7)) PORT(ST7920_D7) |= ST7920_D7_LINE; else PORT(ST7920_D7) &= ~ST7920_D7_LINE;

	return;
}

static void st7920SetDdr(uint8_t data)
{
	if (data & (1<<0)) DDR(ST7920_D0) |= ST7920_D0_LINE; else DDR(ST7920_D0) &= ~ST7920_D0_LINE;
	if (data & (1<<1)) DDR(ST7920_D1) |= ST7920_D1_LINE; else DDR(ST7920_D1) &= ~ST7920_D1_LINE;
	if (data & (1<<2)) DDR(ST7920_D2) |= ST7920_D2_LINE; else DDR(ST7920_D2) &= ~ST7920_D2_LINE;
	if (data & (1<<3)) DDR(ST7920_D3) |= ST7920_D3_LINE; else DDR(ST7920_D3) &= ~ST7920_D3_LINE;
	if (data & (1<<4)) DDR(ST7920_D4) |= ST7920_D4_LINE; else DDR(ST7920_D4) &= ~ST7920_D4_LINE;
	if (data & (1<<5)) DDR(ST7920_D5) |= ST7920_D5_LINE; else DDR(ST7920_D5) &= ~ST7920_D5_LINE;
	if (data & (1<<6)) DDR(ST7920_D6) |= ST7920_D6_LINE; else DDR(ST7920_D6) &= ~ST7920_D6_LINE;
	if (data & (1<<7)) DDR(ST7920_D7) |= ST7920_D7_LINE; else DDR(ST7920_D7) &= ~ST7920_D7_LINE;

	return;
}

static void st7920Write(uint8_t type, uint8_t data)
{
	_delay_us(50);

	if (type == ST7920_DATA)
		PORT(ST7920_RS) |= ST7920_RS_LINE;
	else
		PORT(ST7920_RS) &= ~ST7920_RS_LINE;

	st7920SetPort(data);

	PORT(ST7920_E) |= ST7920_E_LINE;
	asm("nop");
	PORT(ST7920_E) &= ~ST7920_E_LINE;

	return;
}

ISR (TIMER0_OVF_vect)
{
	/* 2MHz / (255 - 155) = 20000Hz => 10kHz Fourier analysis */
	TCNT0 = 155;										/* 20000 / 32 / 34 = 18.4 FPS */

	ADCSRA |= 1<<ADSC;

	static uint8_t i = 0;
	static uint8_t j = 32;

	static uint8_t br;

	if (j == 32) {										/* Phase 1 (Y) */
		PORT(ST7920_RS) &= ~ST7920_RS_LINE;				/* Go to command mode */
		if (++i >= 32)
			i = 0;
		st7920SetPort(ST7920_SET_GRAPHIC_RAM | i);		/* Set Y */
	} else if (j == 33) {								/* Phase 2 (X) */
		st7920SetPort(ST7920_SET_GRAPHIC_RAM);			/* Set X */
	} else {											/* Phase 3 (32 bytes of data) */
		st7920SetPort(fb[j][i]);
	}

	PORT(ST7920_E) |= ST7920_E_LINE;						/* Strob */
	asm("nop");
	PORT(ST7920_E) &= ~ST7920_E_LINE;

	if (++j >= 34) {
		j = 0;
		PORT(ST7920_RS) |= ST7920_RS_LINE;				/* Go to data mode */
	}

	if (++br >= ST7920_MAX_BRIGTHNESS)					/* Loop brightness */
		br = ST7920_MIN_BRIGHTNESS;

	if (br == _br) {
		PORT(ST7920_BCKL) &= ~ST7920_BCKL_LINE;			/* Turn backlight off */
	} else if (br == 0)
		PORT(ST7920_BCKL) |= ST7920_BCKL_LINE;			/* Turn backlight on */

	return;
}

void st7920Init(void)
{
	/* Set control and data lines as outputs */
	DDR(ST7920_RS) |= ST7920_RS_LINE;
	DDR(ST7920_RW) |= ST7920_RW_LINE;
	DDR(ST7920_E) |= ST7920_E_LINE;
	DDR(ST7920_PSB) |= ST7920_PSB_LINE;
	DDR(ST7920_RST) |= ST7920_RST_LINE;
	st7920SetDdr(0xFF);

	PORT(ST7920_RS) &= ~ST7920_RS_LINE;
	PORT(ST7920_RW) &= ~ST7920_RW_LINE;
	PORT(ST7920_E) &= ~ST7920_E_LINE;
	/* Switch display to parallel mode */
	PORT(ST7920_PSB) |= ST7920_PSB_LINE;

	/* Reset display */
	PORT(ST7920_RST) &= ~ST7920_RST_LINE;
	_delay_us(1);
	PORT(ST7920_RST) |= ST7920_RST_LINE;

	/* Init display in graphics mode */
	_delay_ms(40);
	st7920Write(ST7920_COMMAND, ST7920_FUNCTION | ST7920_8BIT);
	st7920Write(ST7920_COMMAND, ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR);
	st7920Write(ST7920_COMMAND, ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR | ST7920_GRAPHIC);

	DDR(ST7920_BCKL)  |= ST7920_BCKL_LINE;

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
