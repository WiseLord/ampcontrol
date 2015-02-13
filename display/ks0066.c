#include "ks0066.h"

#include <util/delay.h>

#if defined(KS0066_WIRE_PCF8574)
static uint8_t i2cData;
#endif

#define swap(x) (__builtin_avr_swap(x))				/* Swaps nibbles */

static void ks0066WriteStrob()
{
#if defined(KS0066_WIRE_PCF8574)
	I2CWriteByte(i2cData | PCF8574_E_LINE);
	I2CWriteByte(i2cData);
#else
	_delay_us(0.04);
	PORT(KS0066_E) |= KS0066_E_LINE;
	_delay_us(0.23);
	PORT(KS0066_E) &= ~KS0066_E_LINE;
#endif

	return;
}

static void ks0066SetData(uint8_t data)
{
#if defined(KS0066_WIRE_PCF8574)
	i2cData &= 0x0F;
	i2cData |= (data & 0xF0);
#else
	if (data & (1<<7)) PORT(KS0066_D7) |= KS0066_D7_LINE; else PORT(KS0066_D7) &= ~KS0066_D7_LINE;
	if (data & (1<<6)) PORT(KS0066_D6) |= KS0066_D6_LINE; else PORT(KS0066_D6) &= ~KS0066_D6_LINE;
	if (data & (1<<5)) PORT(KS0066_D5) |= KS0066_D5_LINE; else PORT(KS0066_D5) &= ~KS0066_D5_LINE;
	if (data & (1<<4)) PORT(KS0066_D4) |= KS0066_D4_LINE; else PORT(KS0066_D4) &= ~KS0066_D4_LINE;
#if defined(KS0066_WIRE_8BIT)
	if (data & (1<<3)) PORT(KS0066_D3) |= KS0066_D3_LINE; else PORT(KS0066_D3) &= ~KS0066_D3_LINE;
	if (data & (1<<2)) PORT(KS0066_D2) |= KS0066_D2_LINE; else PORT(KS0066_D2) &= ~KS0066_D2_LINE;
	if (data & (1<<1)) PORT(KS0066_D1) |= KS0066_D1_LINE; else PORT(KS0066_D1) &= ~KS0066_D1_LINE;
	if (data & (1<<0)) PORT(KS0066_D0) |= KS0066_D0_LINE; else PORT(KS0066_D0) &= ~KS0066_D0_LINE;
#endif
#endif

	return;
}

static void ks0066WritePort(uint8_t data)
{
#if defined(KS0066_WIRE_PCF8574)
	I2CStart(PCF8574_ADDR);
	i2cData &= ~PCF8574_RW_LINE;
#else
	_delay_us(100);
#endif

	ks0066SetData(data);
	ks0066WriteStrob();

#if !defined(KS0066_WIRE_8BIT)
	ks0066SetData(swap(data));
	ks0066WriteStrob();
#endif

#if defined(KS0066_WIRE_PCF8574)
	I2CStop();
#endif

	return;
}

void ks0066WriteCommand(uint8_t command)
{
#if defined(KS0066_WIRE_PCF8574)
	i2cData &= ~PCF8574_RS_LINE;
#else
	PORT(KS0066_RS) &= ~KS0066_RS_LINE;
#endif
	ks0066WritePort(command);

	return;
}

void ks0066WriteData(uint8_t data)
{
#if defined(KS0066_WIRE_PCF8574)
	i2cData |= PCF8574_RS_LINE;
#else
	PORT(KS0066_RS) |= KS0066_RS_LINE;
#endif
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
#if defined(KS0066_WIRE_PCF8574)
	I2CStart(PCF8574_ADDR);

	i2cData |= PCF8574_BL_LINE;
	i2cData &= ~(PCF8574_E_LINE | PCF8574_RW_LINE | PCF8574_RS_LINE);
#else
	DDR(KS0066_D7) |= KS0066_D7_LINE;
	DDR(KS0066_D6) |= KS0066_D6_LINE;
	DDR(KS0066_D5) |= KS0066_D5_LINE;
	DDR(KS0066_D4) |= KS0066_D4_LINE;
#if defined(KS0066_WIRE_8BIT)
	DDR(KS0066_D3) |= KS0066_D3_LINE;
	DDR(KS0066_D2) |= KS0066_D2_LINE;
	DDR(KS0066_D1) |= KS0066_D1_LINE;
	DDR(KS0066_D0) |= KS0066_D0_LINE;
#endif
	DDR(KS0066_E) |= KS0066_E_LINE;
	DDR(KS0066_RW) |= KS0066_RW_LINE;
	DDR(KS0066_RS) |= KS0066_RS_LINE;

	PORT(KS0066_BL) |= KS0066_BL_LINE;
	PORT(KS0066_E) &= ~KS0066_E_LINE;
	PORT(KS0066_RW) &= ~KS0066_RW_LINE;
	PORT(KS0066_RS) &= ~KS0066_RS_LINE;
#endif

	ks0066SetData(KS0066_FUNCTION | KS0066_8BIT);	/* Init data */
	ks0066WriteStrob();
	_delay_ms(20);
	ks0066WriteStrob();
	_delay_ms(5);
	ks0066WriteStrob();
	_delay_us(120);
	ks0066WriteStrob();

#if defined(KS0066_WIRE_PCF8574)
	I2CStop();
#endif

#if defined(KS0066_WIRE_8BIT)
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_8BIT);
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_8BIT | KS0066_2LINES);
#else
	ks0066WriteCommand(KS0066_FUNCTION);
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_2LINES);
#endif
	ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	ks0066WriteCommand(KS0066_CLEAR);
	_delay_ms(2);
	ks0066WriteCommand(KS0066_SET_MODE | KS0066_INC_ADDR);

	return;
}

void ks0066SetXY(uint8_t x, uint8_t y)
{
	ks0066WriteCommand(KS0066_SET_DDRAM + (y ? KS0066_LINE_WIDTH : 0) + x);

	return;
}

void ks0066WriteString(uint8_t *string)
{
	while(*string)
		ks0066WriteData(*string++);

	return;
}

void ks0066WriteHex(uint8_t data)
{
	uint8_t ch;

	ch = (swap(data) & 0x0F) + '0';
	if (ch > '9')
		ch += 'A' - '9' - 1;
	ks0066WriteData(ch);
	ch = (data & 0x0F) + '0';
	if (ch > '9')
		ch += 'A' - '9' - 1;
	ks0066WriteData(ch);

	return;
}

void ks0066SetBacklight(uint8_t value)
{
#if defined(KS0066_WIRE_PCF8574)
	if (value)
		i2cData |= PCF8574_BL_LINE;
	else
		i2cData &= ~PCF8574_BL_LINE;
	ks0066WriteCommand(KS0066_NO_COMMAND);
#endif

	return;
}
