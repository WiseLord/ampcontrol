#include "ks0066.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static void ks0066writeStrob()
{
	asm("nop");	/* 40ns */
	KS0066_CTRL_PORT |= KS0066_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	KS0066_CTRL_PORT &= ~KS0066_E;

	return;
}

static uint8_t ks0066readStrob()
{
	uint8_t pin;

	asm("nop");	/* 40ns */
	KS0066_CTRL_PORT |= KS0066_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	pin = KS0066_DATA_PIN;
	KS0066_CTRL_PORT &= ~KS0066_E;
#ifdef LCD_4BIT_MODE
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	LCD_CONTROL_PORT |= LCD_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	pin &= 0xF0;
	pin |= swap(LCD_DATA_PIN);
	LCD_CONTROL_PORT &= ~LCD_E;
#endif

	return pin;
}

static void ks0066waitWhileBusy()
{
	uint8_t i = 0;

	KS0066_CTRL_PORT &= ~KS0066_RS;
	KS0066_CTRL_PORT |= KS0066_RW;

#ifdef LCD_4BIT_MODE
	LCD_DATA_DDR &= 0x0F;
#else
	KS0066_DATA_DDR = 0x00;
#endif

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

#ifdef LCD_4BIT_MODE
	LCD_DATA_DDR |= 0xF0;
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (command & 0xF0);
	writeStrob();
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (swap(command) & 0xF0);
#else
	KS0066_DATA_DDR |= 0xFF;
	KS0066_DATA_PORT = command;
#endif

	ks0066writeStrob();

	return;
}

void ks0066WriteData(uint8_t data)
{
	ks0066waitWhileBusy();

	KS0066_CTRL_PORT &= ~KS0066_RW;
	KS0066_CTRL_PORT |= KS0066_RS;

#ifdef LCD_4BIT_MODE
	LCD_DATA_DDR |= 0xF0;
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (data & 0xF0);
	writeStrob();
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (swap(data) & 0xF0);
#else
	KS0066_DATA_DDR |= 0xFF;
	KS0066_DATA_PORT = data;
#endif

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

#ifdef LCD_4BIT_MODE
	lcdWriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);
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
