#include "ks0066.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static void writeStrob()
{
	asm("nop");	/* 40ns */
	LCD_CONTROL_PORT |= LCD_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	LCD_CONTROL_PORT &= ~LCD_E;

	return;
}

static uint8_t readStrob()
{
	uint8_t pin;

	asm("nop");	/* 40ns */
	LCD_CONTROL_PORT |= LCD_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	pin = LCD_DATA_PIN;
	LCD_CONTROL_PORT &= ~LCD_E;
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

static void waitWhileBusy()
{
	uint8_t i = 0;

	LCD_CONTROL_PORT &= ~LCD_RS;
	LCD_CONTROL_PORT |= LCD_RW;

#ifdef LCD_4BIT_MODE
	LCD_DATA_DDR &= 0x0F;
#else
	LCD_DATA_DDR = 0x00;
#endif

	while (readStrob() & KS0066_STA_BUSY) {
		if (i++ > 200)	/* Avoid endless loop */
			return;
	}

	return;
}

void lcdWriteCommand(uint8_t command)
{
	waitWhileBusy();

	LCD_CONTROL_PORT &= ~(LCD_RS | LCD_RW);

#ifdef LCD_4BIT_MODE
	LCD_DATA_DDR |= 0xF0;
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (command & 0xF0);
	writeStrob();
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (swap(command) & 0xF0);
#else
	LCD_DATA_DDR |= 0xFF;
	LCD_DATA_PORT = command;
#endif

	writeStrob();

	return;
}

void lcdWriteData(uint8_t data)
{
	waitWhileBusy();

	LCD_CONTROL_PORT &= ~LCD_RW;
	LCD_CONTROL_PORT |= LCD_RS;

#ifdef LCD_4BIT_MODE
	LCD_DATA_DDR |= 0xF0;
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (data & 0xF0);
	writeStrob();
	LCD_DATA_PORT &= 0x0F;
	LCD_DATA_PORT |= (swap(data) & 0xF0);
#else
	LCD_DATA_DDR |= 0xFF;
	LCD_DATA_PORT = data;
#endif

	writeStrob();

	return;
}

void lcdClear(void)
{
	lcdWriteCommand(KS0066_CLEAR);
	_delay_ms(2);

	return;
}

void lcdInit(void)
{
	LCD_DATA_DDR |= 0xFF;
	LCD_CONTROL_DDR |= LCD_RS | LCD_RW | LCD_E;

	LCD_DATA_PORT |= KS0066_INIT_DATA;
	LCD_CONTROL_PORT &= ~(LCD_RS | LCD_RW);
	_delay_ms(20);
	writeStrob();
	_delay_ms(5);
	writeStrob();
	_delay_us(120);
	writeStrob();

#ifdef LCD_4BIT_MODE
	lcdWriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);
#else
	lcdWriteCommand(KS0066_FUNCTION | KS0066_8BIT | KS0066_2LINES);
#endif
	lcdWriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	lcdWriteCommand(KS0066_CLEAR);
	_delay_ms(2);
	lcdWriteCommand(KS0066_SET_MODE | KS0066_INC_ADDR);

	return;
}

void lcdSetXY(uint8_t x, uint8_t y)
{
	lcdWriteCommand(KS0066_SET_DDRAM | (KS0066_LINE_WIDTH * y + x));

	return;
}

void lcdWriteString(uint8_t *string)
{
	while(*string)
		lcdWriteData(*string++);

	return;
}
