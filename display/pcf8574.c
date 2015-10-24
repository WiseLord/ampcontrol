#include "pcf8574.h"

#include <util/delay.h>
#include "../i2c.h"

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static uint8_t portData;

static void ks0066WriteStrob()
{
	I2CWriteByte(portData | PCF8574_E);
	I2CWriteByte(portData);

	return;
}

static void ks0066WritePort(uint8_t data)
{
	I2CStart(PCF8574_ADDR);
	portData &= ~PCF8574_RW;
	portData &= 0x0F;
	portData |= (data & 0xF0);
	ks0066WriteStrob();
	portData &= 0x0F;
	portData |= (swap(data) & 0xF0);
	ks0066WriteStrob();
	I2CStop();

	return;
}

void ks0066WriteCommand(uint8_t command)
{
	portData &= ~PCF8574_RS;
	ks0066WritePort(command);

	return;
}

void ks0066WriteData(uint8_t data)
{
	portData |= PCF8574_RS;
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
	I2CStart(PCF8574_ADDR);
	portData = KS0066_INIT_DATA;
	portData &= ~(PCF8574_RS | PCF8574_RW | PCF8574_E);
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

void pcf8574IntBacklight(uint8_t value)
{
	if (value)
		portData |= PCF8574_BCKL;
	else
		portData &= ~PCF8574_BCKL;

	return;
}
