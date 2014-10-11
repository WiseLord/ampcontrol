#include "ks0066.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static void ks0066writeStrob()
{
	_delay_us(0.04);
	KS0066_CTRL_PORT |= KS0066_E;
	_delay_us(0.23);
	KS0066_CTRL_PORT &= ~KS0066_E;

	return;
}

static void ks0066WritePort(uint8_t data)
{
	_delay_us(100);

#ifdef KS0066_4BIT_MODE
	KS0066_DATA_PORT &= 0x0F;
	KS0066_DATA_PORT |= (data & 0xF0);
	ks0066writeStrob();
	KS0066_DATA_PORT &= 0x0F;
	KS0066_DATA_PORT |= (swap(data) & 0xF0);
#else
	KS0066_DATA_DDR |= 0xFF;
	KS0066_DATA_PORT = data;
#endif
	ks0066writeStrob();

	return;
}

void ks0066WriteCommand(uint8_t command)
{
	KS0066_CTRL_PORT &= ~KS0066_RS;
	ks0066WritePort(command);

	return;
}

void ks0066WriteData(uint8_t data)
{
	KS0066_CTRL_PORT |= KS0066_RS;
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
	KS0066_DATA_DDR |= 0xFF;
	KS0066_CTRL_DDR |= KS0066_RS | KS0066_E;

	KS0066_DATA_PORT |= KS0066_INIT_DATA;
	KS0066_CTRL_PORT &= ~KS0066_RS;
	_delay_ms(20);
	ks0066writeStrob();
	_delay_ms(5);
	ks0066writeStrob();
	_delay_us(120);
	ks0066writeStrob();

#ifdef KS0066_4BIT_MODE
	ks0066WriteCommand(swap(KS0066_FUNCTION | KS0066_4BIT));
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);
#else
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_8BIT | KS0066_2LINES);
#endif
	ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	ks0066WriteCommand(KS0066_CLEAR);
	_delay_ms(2);
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
