#include "ks0108.h"

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

const unsigned char dat[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};

void gdGraph(int16_t *fr)
{
	uint8_t i, j;
	uint8_t row, data;
	int16_t val;
	for (i = 0; i < GD_ROWS; i++)
	{
		gdWrite(GD_COMM, KS0108_SET_PAGE + i, CS1 | CS2);
		gdWrite(GD_COMM, KS0108_SET_ADDRESS, CS1 | CS2);
		for (j = 0; j < GD_COLS * 2; j++)
		{
			val = fr[j>>2];
			row = 8 - (val + 7) / 8;
			data = 0xFF;
			if (i == row)
				data = dat[(val + 7) % 8];
			else if (i < row)
				data = 0x00;
			if (j % 4 == 0)
				data = 0x00;
			if (j < GD_COLS)
				gdWrite(GD_DATA, data, CS1);
			else
				gdWrite(GD_DATA, data, CS2);
		}
	}
	return;
}
