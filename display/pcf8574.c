#include "pcf8574.h"

#include <util/delay.h>
#include "../i2c.h"

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static uint8_t portData;

static void ks0066WriteStrob()
{
	I2CWriteByte(portData | KS0066_E);
	I2CWriteByte(portData);

	return;
}

void ks0066WriteCommand(uint8_t command)
{
	I2CStart(PCF8574_ADDR);
	portData &= ~(KS0066_RS | KS0066_RW);
	portData &= 0x0F;
	portData |= (command & 0xF0);
	ks0066WriteStrob();
	portData &= 0x0F;
	portData |= (swap(command) & 0xF0);
	ks0066WriteStrob();
	I2CStop();

	return;
}

void ks0066WriteData(uint8_t data)
{
	I2CStart(PCF8574_ADDR);
	portData &= ~KS0066_RW;
	portData |= KS0066_RS;
	portData &= 0x0F;
	portData |= (data & 0xF0);
	ks0066WriteStrob();
	portData &= 0x0F;
	portData |= (swap(data) & 0xF0);
	ks0066WriteStrob();
	I2CStop();

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
	I2CStart(PCF8574_ADDR);
	portData = KS0066_INIT_DATA;
	portData &= ~(KS0066_RS | KS0066_RW | KS0066_E);
	ks0066WriteStrob();
	_delay_ms(20);
	ks0066WriteStrob();
	_delay_ms(5);
	ks0066WriteStrob();
	_delay_us(120);
	ks0066WriteStrob();
	I2CStop();

	ks0066WriteCommand(KS0066_FUNCTION >> 4);
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);

	ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	ks0066WriteCommand(KS0066_CLEAR);
	_delay_ms(2);
	ks0066WriteCommand(KS0066_SET_MODE | KS0066_INC_ADDR);

	return;
}

void ks0066SetXY(uint8_t x, uint8_t y)
{
	ks0066WriteCommand(KS0066_SET_DDRAM + (y ? 0x40 : 0x00) + x);

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
		portData |= KS0066_BCKL;
	else
		portData &= ~KS0066_BCKL;
	I2CStart(PCF8574_ADDR);
	I2CWriteByte(portData & KS0066_BCKL);
	I2CStop();

	return;
}
