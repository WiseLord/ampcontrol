#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "ls020.h"
#include "../pins.h"

#define LS020_ROTATE_180

const uint8_t *_font;
static uint8_t fp[FONT_PARAM_COUNT];
static uint8_t _x, _y;
static uint8_t _br;

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

const uint16_t power[26] PROGMEM = {
	0xEF00, 0x7E04, 0xEFB0, 0x5A48,
	0xEF00, 0x7F01, 0xEFB0, 0x64FF,
	0x6500, 0xEF00, 0x7F01, 0xE262,
	0xE202, 0xEFB0, 0xBC02, 0xEF00,
	0x7F01, 0xE200, 0x8000, 0xE204,
	0xE200, 0xE100, 0xEFB0, 0xBC00,
	0xEF00, 0x7F01
};

static void ls020WriteCommand(uint16_t command)
{ 
	PORT(LS020_DPORT) |= LS020_RS_LINE;
	PORT(LS020_DPORT) &= ~LS020_CS_LINE;

	SPDR = command >> 8;
	while(!(SPSR & (1<<SPIF)));
	SPDR = command & 0xFF;
	while(!(SPSR & (1<<SPIF)));

	PORT(LS020_DPORT) |= LS020_CS_LINE;

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
#ifdef LS020_ROTATE_180
	ls020WriteCommand(0x0800 + LS020_WIDTH - 1 - y1);
	ls020WriteCommand(0x0900 + LS020_HEIGHT - 1 - y0);
	ls020WriteCommand(0x0A00 + x0);
	ls020WriteCommand(0x0B00 + x1);
#else
	ls020WriteCommand(0x0800 + y0);
	ls020WriteCommand(0x0900 + y1);
	ls020WriteCommand(0x0A00 + LS020_HEIGHT - 1 - x1);
	ls020WriteCommand(0x0B00 + LS020_HEIGHT - 1 - x0);
#endif
}

void ls020DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
	uint8_t x, y;

	/* Swap X */
	if (x1 > x2) {
		x = x1;
		x1 = x2;
		x2 = x;
	}

	/* Swap Y */
	if (y1 > y2) {
		y = y1;
		y1 = y2;
		y2 = y;
	}

	if (x2 >= LS020_HEIGHT)
		return;
	if (y2 >= LS020_WIDTH)
		return;

	ls020SetWindow(x1, y1, x2, y2);

	PORT(LS020_DPORT) &= ~LS020_RS_LINE;
	PORT(LS020_DPORT) &= ~LS020_CS_LINE;
	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			ls020WriteData(color);
	PORT(LS020_DPORT) |= LS020_CS_LINE;

	return;
}

void ls020DrawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
	ls020DrawHorizLine(x1, y1, x2, color);
	ls020DrawHorizLine(x1, y2, x2, color);
	ls020DrawVertLine(x1, y1, y2, color);
	ls020DrawVertLine(x2, y1, y2, color);

	return;
}

void ls020Clear(void)
{
	ls020DrawRect(0, 0, 175, 131, COLOR_BCKG);

	return;
}

ISR (TIMER0_OVF_vect)
{
	/* 2MHz / (256 - 156) = 20000Hz */
	TCNT0 = 156;

	static uint8_t run = 1;
	if (run)
		ADCSRA |= 1<<ADSC;							/* Start ADC every second interrupt */
	run = !run;

	static uint8_t br;

	if (++br >= LS020_MAX_BRIGHTNESS)				/* Loop brightness */
		br = LS020_MIN_BRIGHTNESS;

	if (br == _br) {
		PORT(LS020_BCKL) &= ~LS020_BCKL_LINE;		/* Turn backlight off */
	} else if (br == 0)
		PORT(LS020_BCKL) |= LS020_BCKL_LINE;		/* Turn backlight on */
}


void ls020SetBrightness(uint8_t br)
{
	_br = br;

	return;
}

void ls020Init(void)
{
	uint8_t i;

	DDR(LS020_DPORT) = LS020_RS_LINE | LS020_RES_LINE | LS020_DAT_LINE | LS020_CLK_LINE | LS020_CS_LINE;
	DDR(LS020_BCKL) |= LS020_BCKL_LINE;

	/* Configure Hardware SPI */
	SPCR = (1<<SPE) | (1<<MSTR);
	SPSR = (1<<SPI2X);

	PORT(LS020_DPORT) &= ~LS020_CS_LINE;

	/* Reset display */
	PORT(LS020_DPORT) &= ~LS020_RES_LINE;
	_delay_ms(5);
	PORT(LS020_DPORT) |= LS020_RES_LINE;

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

	PORT(LS020_DPORT) |= LS020_CS_LINE;

	/* Set 8-bit color mode */
	ls020WriteCommand(0xE800);

	ls020Clear();

	return;
}

void ls020PowerOff(void)
{
	uint8_t i;

	for (i = 0; i < 26; i++)
		ls020WriteCommand(pgm_read_word(power + i));

	return;
}

void ls020SetXY(uint8_t x, uint8_t y)
{
	_x = x;
	_y = y;
}

uint8_t ls020GetX(void)
{
	return _x;
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

	PORT(LS020_DPORT) &= ~LS020_RS_LINE;
	PORT(LS020_DPORT) &= ~LS020_CS_LINE;
	for (k = 0; k < fp[FONT_HEIGHT]; k++) {
		for (j = 0; j < swd; j++) {
#ifdef LS020_ROTATE_180
			pgmData[j] = pgm_read_word(_font + oft + ((fp[FONT_HEIGHT] - k - 1)) * swd + j);
#else
			pgmData[j] = pgm_read_word(_font + oft + (k + 1) * swd - j - 1);
#endif
		}
		for(i = 0; i < 8; i++) {
			for (my = 0; my < fp[FONT_MULT]; my++) {
				for (j = 0; j < swd; j++) {
#ifdef LS020_ROTATE_180
					if (pgmData[j] & (128>>i)) {
#else
					if (pgmData[j] & (1<<i)) {
#endif
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
	PORT(LS020_DPORT) |= LS020_CS_LINE;

	ls020SetXY(x + swd * fp[FONT_MULT], y);

	return;
}

void ls020WriteString(char *string)
{
	if (*string)
		ls020WriteChar(*string++);
	while(*string) {
		ls020WriteChar(fp[FONT_LTSPPOS]);
		ls020WriteChar(*string++);
	}

	return;
}

void ls020WriteIcon24(uint8_t iconNum)
{
	uint8_t i, j, k;
	uint8_t pgmData;

	const uint8_t *icon;

	icon = &icons_24[24 * 24 / 8 * iconNum];

	if (icon) {
		for (j = 0; j < 3; j++) {
			for (i = 0; i < 24; i++) {
				pgmData = pgm_read_byte(icon + 24 * j + i);
				for (k = 0; k < 8; k++) {
					ls020DrawRect(_x + i, _y + 8 * j + k, _x + i, _y + 8 * j + k, pgmData & (1<<k) ? COLOR_WHITE : COLOR_BLACK);
				}
			}
		}
	}

	return;
}

void ls020WriteIcon32(uint8_t iconNum)
{
	uint8_t i, j, k;
	uint8_t pgmData;

	const uint8_t *icon;

	icon = &icons_32[32 * 32 / 8 * iconNum];

	if (icon) {
		for (j = 0; j < 4; j++) {
			for (i = 0; i < 32; i++) {
				pgmData = pgm_read_byte(icon + 32 * j + i);
				for (k = 0; k < 8; k++) {
					ls020DrawRect(_x + i, _y + 8 * j + k, _x + i, _y + 8 * j + k, pgmData & (1<<k) ? COLOR_YELLOW : COLOR_BLACK);
				}
			}
		}
	}

	return;
}
