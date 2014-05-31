#ifndef LS020_H
#define LS020_H

#include <avr/io.h>
#include <inttypes.h>

#include "fonts.h"

#define LS020_HEIGHT	176
#define LS020_WIDTH		132

/* Color bit map		0bRRRGGGBB */
#define COLOR_RED		0b11100000
#define COLOR_GREEN		0b00011100
#define COLOR_BLUE		0b00000011
#define COLOR_YELLOW	0b11111100
#define COLOR_CYAN		0b00011111
#define COLOR_MAGENTA	0b11100011
#define COLOR_BLACK		0b00000000
#define COLOR_WHITE		0b11111111

#define COLOR_BCKG		COLOR_BLACK

#define LS020_DDR		DDRB
#define LS020_PORT		PORTB

#define LS020_RS		(1<<PB1)
#define LS020_RES		(1<<PB0)
#define LS020_CS		(1<<PB4) /* !SS */
#define LS020_CLK		(1<<PB7) /* SCK */
#define LS020_DAT		(1<<PB5) /* MOSI */

#define FONT_PARAM_COUNT			7
enum {
	FONT_HEIGHT,
	FONT_LTSPPOS,
	FONT_CCNT,
	FONT_OFTA,
	FONT_OFTNA,
	FONT_COLOR,
	FONT_MULT
};

void ls020Init();

void ls020PowerOff (void);

void ls020WriteCommand(uint16_t command);

void ls020WriteData(uint8_t data);

void ls020DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

void ls020FillScreen(uint8_t color);

void ls020SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

void ls020SetXY(uint8_t x, uint8_t y);

void ls020LoadFont(const uint8_t *font, uint8_t color, uint8_t mult);

void ls020WriteChar(uint8_t code);

void ls020WriteString(uint8_t *string);

void ls020DrawImage(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint16_t *pic);

void ls020DrawImageMono(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *pic);

#endif /* _LCD_H */
