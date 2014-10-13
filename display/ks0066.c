#include "ks0066.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static void ks0066writeStrob()
{
	_delay_us(0.04);
	PORT(KS0066_E) |= KS0066_E_LINE;
	_delay_us(0.23);
	PORT(KS0066_E) &= ~KS0066_E_LINE;

	return;
}

static void ks0066SetHighNibble(uint8_t data)
{
	if (data & (1<<7)) PORT(KS0066_D7) |= KS0066_D7_LINE; else PORT(KS0066_D7) &= ~KS0066_D7_LINE;
	if (data & (1<<6)) PORT(KS0066_D6) |= KS0066_D6_LINE; else PORT(KS0066_D6) &= ~KS0066_D6_LINE;
	if (data & (1<<5)) PORT(KS0066_D5) |= KS0066_D5_LINE; else PORT(KS0066_D5) &= ~KS0066_D5_LINE;
	if (data & (1<<4)) PORT(KS0066_D4) |= KS0066_D4_LINE; else PORT(KS0066_D4) &= ~KS0066_D4_LINE;

	return;
}

static void ks0066WritePort(uint8_t data)
{
	_delay_us(100);

	ks0066SetHighNibble(data);
	ks0066writeStrob();
	ks0066SetHighNibble(swap(data));
	ks0066writeStrob();

	return;
}

void ks0066WriteCommand(uint8_t command)
{
	PORT(KS0066_RS) &= ~KS0066_RS_LINE;
	ks0066WritePort(command);

	return;
}

void ks0066WriteData(uint8_t data)
{
	PORT(KS0066_RS) |= KS0066_RS_LINE;
	ks0066WritePort(data);

	return;
}

void ks0066Clear(void)
{
	ks0066WriteCommand(KS0066_CLEAR);
	_delay_ms(2);

	return;
}

void ks0066Init(void)
{
	DDR(KS0066_D7) |= KS0066_D7_LINE;
	DDR(KS0066_D6) |= KS0066_D6_LINE;
	DDR(KS0066_D5) |= KS0066_D5_LINE;
	DDR(KS0066_D4) |= KS0066_D4_LINE;
	DDR(KS0066_RS) |= KS0066_RS_LINE;
	DDR(KS0066_E) |= KS0066_E_LINE;

	ks0066SetHighNibble(KS0066_INIT_DATA);
	PORT(KS0066_RS) &= ~KS0066_RS_LINE;
	_delay_ms(20);
	ks0066writeStrob();
	_delay_ms(5);
	ks0066writeStrob();
	_delay_us(120);
	ks0066writeStrob();

	ks0066WriteCommand(swap(KS0066_FUNCTION | KS0066_4BIT));
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);
	ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	ks0066Clear();
	ks0066WriteCommand(KS0066_SET_MODE | KS0066_INC_ADDR);

	return;
}

void ks0066SetXY(uint8_t x, uint8_t y)
{
	ks0066WriteCommand(KS0066_SET_DDRAM | (KS0066_LINE_WIDTH * y + x));

	return;
}

void ks0066WriteString(uint8_t *string)
{
	while(*string)
		ks0066WriteData(*string++);

	return;
}
