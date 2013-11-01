#include "ks0108.h"
#include "font.h"

#include <util/delay.h>

void gdWrite(uint8_t mode, uint8_t data, uint8_t cs)
{
	GD_CPORT |= cs;
	GD_DDDR = 0xFF;
	GD_DPORT = data;

	if (mode) GD_CPORT |= DI; else GD_CPORT &= ~DI;

	GD_CPORT |= E;
	asm("nop");
	GD_CPORT &= ~E;

	GD_DDDR = 0x00;
	GD_CPORT &= ~cs;
	_delay_us(7);

	return;
}

void gdFill(uint8_t data, uint8_t cs)
{
	GD_CPORT |= cs;
	uint8_t i, j;
	for (i = 0; i < GD_ROWS; i++)
	{
		gdWrite(GD_COMM, KS0108_SET_PAGE + i, cs);
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, cs);
		for (j = 0; j < GD_COLS; j++)
			gdWrite(GD_DATA, data, cs);
	}
	GD_CPORT &= ~cs;
	return;
}

void gdInit(void)
{
	GD_DDDR = 0x00;
	GD_DPORT = 0x00;

	GD_CDDR |= CS1 | CS2 | RES | E | RW | DI;

	GD_CPORT &= RES;
	_delay_us(10);
	GD_CPORT |= RES;

	GD_CPORT &= ~(CS1 | CS2 | E | RW | DI);

	gdFill(0x00, CS1 | CS2);

	gdWrite(GD_COMM, KS0108_DISPLAY_ON, CS1 | CS2);
	gdWrite(GD_COMM, KS0108_DISPLAY_START_LINE, CS1 | CS2);

	return;
}

void gdSpectrum(uint8_t *buf, uint8_t mode)
{
	uint8_t i, j;
	int8_t row;
	uint8_t data;
	uint8_t val;
	for (i = 0; i < GD_ROWS; i++)
	{
		gdWrite(GD_COMM, KS0108_SET_PAGE + i, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		for (j = 0; j < GD_COLS * 2; j++)
		{
			switch (mode)
			{
			case MODE_LEFT:
				val = buf[j>>2] << 1;
				row = 7 - val / 8;
				break;
			case MODE_RIGHT:
				val = buf[(j>>2) + 32] << 1;
				row = 7 - val / 8;
				break;
			case MODE_STEREO:
				if (i < GD_ROWS / 2)
				{
					val = buf[j>>2];
					row = 3 - val / 8;
				}
				else
				{
					val = buf[(j>>2) + 32];
					row = 7 - val / 8;
				}
				break;
			default:
				val = buf[j>>2] + buf[(j>>2) + 32];
				row = 7 - val / 8;
				break;
			}
			data = 0xFF;
			if (i == row)
				data = 0xFF << (7 - val % 8);
			else if (i < row)
				data = 0x00;
			if (!(j & 0x03))
				data = 0x00;
			if (j < GD_COLS)
				gdWrite(GD_DATA, data, CS1);
			else
				gdWrite(GD_DATA, data, CS2);
		}
	}
	return;
}

static uint8_t xPos, yPos;

void gdSetPos(uint8_t x, uint8_t y)
{
	uint8_t cs;
	cs = CS1;
	if (x >= GD_COLS)
		cs = CS2;
	gdWrite(GD_COMM, KS0108_SET_ADDRESS + (x & (GD_COLS - 1)), cs);
	xPos = x;
	gdWrite(GD_COMM, KS0108_SET_PAGE + (y >> 3), cs);
	yPos = y;
	return;
}

void gdWriteChar(uint8_t code)
{
	uint8_t cs;
	cs = CS1;
	if (xPos >= GD_COLS)
		cs = CS2;
	uint8_t i;
	uint16_t index;
	index = code * 5;
	for (i = 0; i < 6; i++)
	{
		if (i == 5)
			gdWrite(GD_DATA, 0x00, cs);
		else
			gdWrite(GD_DATA, pgm_read_byte(&k1013vg6_0[index + i]), cs);
		xPos++;
		if (xPos == GD_COLS)
			cs = CS2;
		if (xPos == GD_COLS * 2)
		{
			cs = CS1;
			xPos = 0;
			yPos += 8;
			if (yPos >= 8 * GD_ROWS)
				yPos /= (8 * GD_ROWS);
		}
		gdSetPos(xPos, yPos);
	}
	return;
}

void gdWriteString(uint8_t *string)
{
	while(*string)
		gdWriteChar(*string++);
	return;
}

void gdWriteNum(int16_t number, uint8_t width)
{
	uint8_t dig[width + 1];
	int8_t i;
	uint8_t sign = ' ';
	if (number < 0)
	{
		sign = '-';
		number = -number;
	}
	for (i = 0; i < width; i++)
		dig[i] = ' ';
	dig[width] = '\0';


	i = width - 1;
	while (number > 0 || i == width - 1)
	{
		dig[i--] = number % 10 + 0x30;
		number /= 10;
	}
	if (i >= 0)
		dig[i] = sign;
	gdWriteString(dig);
}
