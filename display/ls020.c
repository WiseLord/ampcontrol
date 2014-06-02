#include <util/delay.h>
#include <avr/pgmspace.h>

#include "ls020.h"

const uint8_t *_font;
static uint8_t fp[FONT_PARAM_COUNT];
static uint8_t _x, _y;

const uint16_t init1[2] PROGMEM = {
	0xFDFD, 0xFDFD
};

const uint16_t init2[10] PROGMEM = {
	0xEF00, 0xEE04, 0x1B04, 0xFEFE,
	0xFEFE, 0xEF90, 0x4A04, 0x7F3F,
	0xEE04, 0x4306
};

const uint16_t init3[20] PROGMEM = {
	0xEF90, 0x0983, 0x0800, 0x0BAF,
	0x0A00, 0x0500, 0x0600, 0x0700,
	0xEF00, 0xEE0C, 0xEF90, 0x0080,
	0xEFB0, 0x4902, 0xEF00, 0x7F01,
	0xE181, 0xE202, 0xE276, 0xE183
};

const uint16_t init4[3] PROGMEM = {
	0x8001, 0xEF90, 0x0000
};

//const uint16_t power[26] PROGMEM = {
//	0xEF00, 0x7E04, 0xEFB0, 0x5A48,
//	0xEF00, 0x7F01, 0xEFB0, 0x64FF,
//	0x6500, 0xEF00, 0x7F01, 0xE262,
//	0xE202, 0xEFB0, 0xBC02, 0xEF00,
//	0x7F01, 0xE200, 0x8000, 0xE204,
//	0xE200, 0xE100, 0xEFB0, 0xBC00,
//	0xEF00, 0x7F01
//};

static void ls020WriteCommand(uint16_t command)
{ 
	LS020_PORT |= LS020_RS;
	LS020_PORT &= ~LS020_CS;

	SPDR = command >> 8;
	while(!(SPSR & (1<<SPIF)));
	SPDR = command & 0xFF;
	while(!(SPSR & (1<<SPIF)));

	LS020_PORT |= LS020_CS;

	return;
}

static void ls020WriteData(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));

	return;
}

static void ls020SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	ls020WriteCommand(0x0504); /* Set Direction */
	ls020WriteCommand(0x0800 + y0);
	ls020WriteCommand(0x0900 + y1);
	ls020WriteCommand(0x0A00 + LS020_HEIGHT - 1 - x1);
	ls020WriteCommand(0x0B00 + LS020_HEIGHT - 1 - x0);
}

void ls020DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
	uint8_t x, y;

	ls020SetWindow(x1, y1, x2, y2);

	LS020_PORT &= ~LS020_RS;
	LS020_PORT &= ~LS020_CS;
	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			ls020WriteData(color);
	LS020_PORT |= LS020_CS;

	return;
}

void ls020FillScreen(uint8_t color)
{
	ls020DrawRect(0, 0, 175, 131, color);

	return;
}

void ls020Init(void)
{
	uint8_t i;

	LS020_DDR = LS020_RS | LS020_RES | LS020_DAT | LS020_CLK | LS020_CS;

	/* Configure Hardware SPI */
	SPCR = (1<<SPE) | (1<<MSTR);
	SPSR = (1<<SPI2X);

	LS020_PORT &= ~LS020_CS;

	/* Reset display */
	LS020_PORT &= ~LS020_RES;
	_delay_ms(5);
	LS020_PORT |= LS020_RES;

	/* Display init magic */
	for (i = 0; i < 2; i++)
		ls020WriteCommand(pgm_read_word(init1 + i));
	_delay_ms(50);
	for (i = 0; i < 10; i++)
		ls020WriteCommand(pgm_read_word(init2 + i));
	_delay_ms(7);
	for (i = 0; i < 20;i++)
		ls020WriteCommand(pgm_read_word(init3 + i));
	_delay_ms(50);
	for (i = 0; i < 3; i++)
		ls020WriteCommand(pgm_read_word(init4 + i));

	LS020_PORT |= LS020_CS;

	/* Set 8-bit color mode */
	ls020WriteCommand(0xE800);

	ls020FillScreen(COLOR_BCKG);

	return;
}

//void ls020PowerOff(void)
//{
//	uint8_t i;

//	for (i = 0; i < 26; i++)
//		ls020WriteCommand(pgm_read_word(power + i));

//	return;
//}

void ls020SetXY(uint8_t x, uint8_t y)
{
	_x = x;
	_y = y;
}

void ls020LoadFont(const uint8_t *font, uint8_t color, uint8_t mult)
{
	uint8_t i;

	_font = font + 5;
	for (i = 0; i < FONT_PARAM_COUNT - 2; i++)
		fp[i] = pgm_read_byte(font + i);
	fp[FONT_COLOR] = color;
	fp[FONT_MULT] = mult;
}

void ls020WriteChar(uint8_t code)
{
	/* Store current position before writing to display */
	uint8_t x = _x;
	uint8_t y = _y;

	uint8_t i, j, k;
	uint8_t mx, my;

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

	ls020SetWindow(x, y, x + swd * fp[FONT_MULT] - 1, y + fp[FONT_HEIGHT] * fp[FONT_MULT] * 8 - 1);

	uint8_t pgmData[swd];

	LS020_PORT &= ~LS020_RS;
	LS020_PORT &= ~LS020_CS;
	for (k = 0; k < fp[FONT_HEIGHT]; k++) {
		for (j = 0; j < swd; j++) {
			pgmData[j] = pgm_read_word(_font + oft + (k + 1) * swd - j - 1);
		}
		for(i = 0; i < 8; i++) {
			for (my = 0; my < fp[FONT_MULT]; my++) {
				for (j = 0; j < swd; j++) {
					if (pgmData[j] & (1<<i)) {
						for (mx = 0; mx < fp[FONT_MULT]; mx++)
							ls020WriteData(fp[FONT_COLOR]);
					} else {
						for (mx = 0; mx < fp[FONT_MULT]; mx++)
							ls020WriteData(COLOR_BCKG);
					}
				}
			}
		}
	}
	LS020_PORT |= LS020_CS;

	ls020SetXY(x + swd * fp[FONT_MULT], y);

	return;
}

void ls020WriteString(uint8_t *string)
{
	if (*string)
		ls020WriteChar(*string++);
	while(*string) {
		ls020WriteChar(fp[FONT_LTSPPOS]);
		ls020WriteChar(*string++);
	}

	return;
}
