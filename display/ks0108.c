#include "ks0108.h"

#include <avr/pgmspace.h>

const uint8_t *_font;
static uint8_t _cs, _row, _col;
static uint8_t csInv;		/* 0 for WG12864A, 1 for WG12864B */

static uint8_t fp[FONT_PARAM_COUNT];

static inline void ks0108SetPortCS()
{
	if (csInv) {
		KS0108_CHIP_PORT |= (KS0108_CS1 | KS0108_CS2);
		KS0108_CHIP_PORT &= ~_cs;
	} else {
		KS0108_CHIP_PORT &= ~(KS0108_CS1 | KS0108_CS2);
		KS0108_CHIP_PORT |= _cs;
	}
	return;
}

static uint8_t ks0108Strob()
{
	uint8_t pin;

	asm("nop");	/* 120ns */
	asm("nop");
	KS0108_CTRL_PORT |= KS0108_E;
	asm("nop");	/* 300ns */
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	pin = KS0108_DATA_PIN;
	KS0108_CTRL_PORT &= ~KS0108_E;

	return pin;
}

uint8_t ks0108ReadStatus()
{
	uint8_t status;

	KS0108_DATA_DDR = 0x00;

	ks0108SetPortCS();

	KS0108_CTRL_PORT |= KS0108_RW;
	KS0108_CTRL_PORT &= ~KS0108_DI;

	status = ks0108Strob();

	return status;
}

void ks0108WaitWhile(uint8_t status)
{
	uint8_t i = 0;
	while(ks0108ReadStatus() & status) {
		if (i++ > 200)	/* Avoid endless loop */
			return;
	}
	return;
}

uint8_t ks0108ReadData()
{
	uint8_t data;

	ks0108WaitWhile(KS0108_STA_BUSY);
	KS0108_CTRL_PORT |= KS0108_DI;

	ks0108Strob();

	ks0108WaitWhile(KS0108_STA_BUSY);
	KS0108_CTRL_PORT |= KS0108_DI;

	data = ks0108Strob();

	return data;
}

void ks0108WriteCommand(uint8_t command)
{
	ks0108WaitWhile(KS0108_STA_BUSY);

	KS0108_DATA_DDR = 0xFF;

	ks0108SetPortCS();

	KS0108_CTRL_PORT &= ~KS0108_RW;
	KS0108_CTRL_PORT &= ~KS0108_DI;

	KS0108_DATA_PORT = command;

	ks0108Strob();

	return;
}

void ks0108WriteData(uint8_t data)
{
	ks0108WaitWhile(KS0108_STA_BUSY);

	KS0108_DATA_DDR = 0xFF;

	ks0108SetPortCS();

	KS0108_CTRL_PORT &= ~KS0108_RW;
	KS0108_CTRL_PORT |= KS0108_DI;

	KS0108_DATA_PORT = data;

	ks0108Strob();

	if (++_col == 64) {
		_col = 0;

		if (_cs & KS0108_CS2)
			_row += fp[FONT_HEIGHT];
		if (_row == KS0108_ROWS)
			_row = 0;

		if (_cs == KS0108_CS1)
			_cs = KS0108_CS2;
		else if (_cs == KS0108_CS2)
			_cs = KS0108_CS1;

		ks0108WriteCommand(KS0108_SET_ADDRESS);
		ks0108WriteCommand(KS0108_SET_PAGE + _row);
	}
	return;
}

void ks0108Fill(uint8_t data)
{
	uint8_t i, j;
	uint8_t cs = _cs;

	_cs = KS0108_CS1 | KS0108_CS2;
	ks0108WriteCommand(KS0108_SET_ADDRESS + _col);
	ks0108WriteCommand(KS0108_SET_PAGE + _row);

	for (i = 0; i < KS0108_ROWS; i++)
		for (j = 0; j < KS0108_COLS; j++)
			ks0108WriteData(data);
	_cs = cs;

	return;
}

void ks0108TestDisplay() {
	const uint8_t wrData[] = {0x55, 0xAA};
	uint8_t rdData[] = {0, 0};
	uint8_t i;

	csInv = 0;

	for (i = 0; i < 2; i++) {
		ks0108WriteCommand(KS0108_SET_ADDRESS);
		ks0108WriteCommand(KS0108_SET_PAGE);
		ks0108WriteData(wrData[i]);

		ks0108WriteCommand(KS0108_SET_ADDRESS);
		ks0108WriteCommand(KS0108_SET_PAGE);
		rdData[i] = ks0108ReadData();
	}

	if (wrData[0] != rdData[0] || wrData[1] != rdData[1])
		csInv = 1;

	return;
}

void ks0108Init(void)
{
	/* Set control lines as outputs */
	KS0108_CTRL_DDR |= KS0108_DI | KS0108_RW | KS0108_E;
	KS0108_CHIP_DDR |= KS0108_CS1 | KS0108_CS2 | KS0108_RES;

	/* Reset */
	KS0108_CHIP_PORT &= ~(KS0108_RES);
	asm("nop");
	asm("nop");
	KS0108_CHIP_PORT |= KS0108_RES;
	asm("nop");
	asm("nop");

	/* Clear display  and reset addresses */
	_cs = KS0108_CS1 | KS0108_CS2;

	ks0108TestDisplay();

	ks0108WriteCommand(KS0108_DISPLAY_START_LINE);
	ks0108WriteCommand(KS0108_SET_ADDRESS);
	ks0108WriteCommand(KS0108_SET_PAGE);

	fp[FONT_HEIGHT] = 1;
	ks0108Fill(0x00);

	ks0108WriteCommand(KS0108_DISPLAY_ON);

	_row = 0;
	_col = 0;
	_cs = KS0108_CS2;

	return;
}

void ks0108SetXY(uint8_t x, uint8_t y)
{
	if (x > ((KS0108_COLS << 1) - 1))
		y += fp[FONT_HEIGHT];

	if ((x & ((KS0108_COLS << 1) - 1)) < KS0108_COLS)
		_cs = KS0108_CS1;
	else
		_cs = KS0108_CS2;

	_col = x & (KS0108_COLS - 1);
	_row = y & (KS0108_ROWS - 1);

	ks0108WriteCommand(KS0108_SET_ADDRESS + _col);
	ks0108WriteCommand(KS0108_SET_PAGE + _row);

}

void ks0108LoadFont(const uint8_t *font, uint8_t color)
{
	uint8_t i;

	_font = font + 5;
	for (i = 0; i < FONT_PARAM_COUNT - 1; i++)
		fp[i] = pgm_read_byte(font + i);
	fp[FONT_COLOR] = color;
}

void ks0108WriteChar(uint8_t code)
{
	/* Store current position before writing to display */
	uint8_t cs = _cs;
	uint8_t row = _row;
	uint8_t col = _col;
	if (cs == KS0108_CS2)
		col += KS0108_COLS;

	uint8_t i, j;

	uint8_t pgmData;

	uint8_t spos = code - ((code >= 128) ? fp[FONT_OFTNA] : fp[FONT_OFTA]);

	uint16_t oft = 0;	/* Current symbol offset in array*/
	uint8_t swd = 0;	/* Current symbol width */

	for (i = 0; i < spos; i++) {
		swd = pgm_read_byte(_font + i);
		oft += swd;
	}
	swd = pgm_read_byte(_font + spos);

	oft *= fp[FONT_HEIGHT];
	oft += fp[FONT_CCNT];

	for (j = 0; j < fp[FONT_HEIGHT]; j++) {
		ks0108SetXY(col, row + j);
		for (i = 0; i < swd; i++) {
			pgmData = pgm_read_byte(_font + oft + (swd * j) + i);
			if (fp[FONT_COLOR])
				ks0108WriteData(pgmData);
			else
				ks0108WriteData(~pgmData);
		}
	}

	ks0108SetXY(col + swd, row);

	return;
}

void ks0108WriteString(uint8_t *string)
{
	if (*string)
		ks0108WriteChar(*string++);
	while(*string) {
		ks0108WriteChar(fp[FONT_LTSPPOS]);
		ks0108WriteChar(*string++);
	}

	return;
}
