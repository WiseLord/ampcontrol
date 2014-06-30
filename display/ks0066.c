#include "ks0066.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static void ks0066writeStrob()
{
	_delay_us(0.05);
	KS0066_CTRL_PORT |= KS0066_E;
	_delay_us(0.25);
	KS0066_CTRL_PORT &= ~KS0066_E;

	return;
}

static uint8_t ks0066readStrob()
{
	uint8_t pin;

	_delay_us(0.05);
	KS0066_CTRL_PORT |= KS0066_E;
	_delay_us(0.25);
	pin = swap(KS0066_DATA_PIN & 0x0F);
	KS0066_CTRL_PORT &= ~KS0066_E;
	_delay_us(0.25);
	KS0066_CTRL_PORT |= KS0066_E;
	_delay_us(0.25);
	pin |= (KS0066_DATA_PIN & 0x0F);
	KS0066_CTRL_PORT &= ~KS0066_E;

	return pin;
}

static void ks0066waitWhileBusy()
{
	uint8_t i = 0;

	KS0066_CTRL_PORT &= ~KS0066_RS;
	KS0066_CTRL_PORT |= KS0066_RW;

	KS0066_DATA_DDR &= 0x0F;

	while (ks0066readStrob() & KS0066_STA_BUSY) {
		if (i++ > 200)	/* Avoid endless loop */
			return;
	}

	return;
}

void ks0066WriteCommand(uint8_t command)
{
	ks0066waitWhileBusy();

	KS0066_CTRL_PORT &= ~(KS0066_RS | KS0066_RW);

	KS0066_DATA_DDR |= 0xF0;
	KS0066_DATA_PORT &= 0x0F;
	KS0066_DATA_PORT |= (command & 0xF0);
	ks0066writeStrob();
	KS0066_DATA_PORT &= 0x0F;
	KS0066_DATA_PORT |= (swap(command) & 0xF0);

	ks0066writeStrob();

	return;
}

void ks0066WriteData(uint8_t data)
{
	ks0066waitWhileBusy();

	KS0066_CTRL_PORT &= ~KS0066_RW;
	KS0066_CTRL_PORT |= KS0066_RS;

	KS0066_DATA_DDR |= 0xF0;
	KS0066_DATA_PORT &= 0x0F;
	KS0066_DATA_PORT |= (data & 0xF0);
	ks0066writeStrob();
	KS0066_DATA_PORT &= 0x0F;
	KS0066_DATA_PORT |= (swap(data) & 0xF0);

	ks0066writeStrob();

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
	KS0066_CTRL_DDR |= KS0066_RS | KS0066_RW | KS0066_E;

	KS0066_DATA_PORT |= KS0066_INIT_DATA;
	KS0066_CTRL_PORT &= ~(KS0066_RS | KS0066_RW);
	_delay_ms(20);
	ks0066writeStrob();
	_delay_ms(5);
	ks0066writeStrob();
	_delay_us(120);
	ks0066writeStrob();

	ks0066WriteCommand(swap(KS0066_FUNCTION | KS0066_4BIT));
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);

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

void ks0066Backlight(uint8_t bckl)
{
	if (bckl)
		KS0066_BCKL_PORT |= KS0066_BCKL;
	else
		KS0066_BCKL_PORT &= ~KS0066_BCKL;

	return;
}
