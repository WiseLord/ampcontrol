#ifndef LS020_H
#define LS020_H

#include <avr/io.h>
#include <inttypes.h>

#include "fonts.h"
#include "icons.h"

#define LS020_HEIGHT			176
#define LS020_WIDTH				132

#define LS020_MIN_BRIGHTNESS	0
#define LS020_MAX_BRIGHTNESS	32

// Color bit map				0bRRRGGGBB
#define COLOR_RED				0b11100000
#define COLOR_GREEN				0b00011100
#define COLOR_BLUE				0b00000011

#define COLOR_YELLOW			0b11111100
#define COLOR_CYAN				0b00011111
#define COLOR_MAGENTA			0b11100011

#define COLOR_DARKRED			0b10000000
#define COLOR_DARKGREEN			0b00010000
#define COLOR_DARKBLUE			0b00000010

#define COLOR_LIME				0b00111100
#define COLOR_ORANGERED			0b11101000
#define COLOR_DODGERBLUE		0b00001011

#define COLOR_DARKYELLOW		0b10010000
#define COLOR_DARKCYAN			0b00010010
#define COLOR_DARKMAGENTA		0b10000010

#define COLOR_BLACK				0b00000000
#define COLOR_GRAY				0b10010010
#define COLOR_WHITE				0b11111111

#define COLOR_BCKG				COLOR_BLACK

#define FONT_PARAM_COUNT		7
enum {
	FONT_HEIGHT = 0,
	FONT_LTSPPOS,
	FONT_CCNT,
	FONT_OFTA,
	FONT_OFTNA,
	FONT_COLOR,
	FONT_MULT
};

void ls020SetBrightness(uint8_t br);

void ls020Init();

void ls020PowerOff (void);

void ls020DrawRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
void ls020DrawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
#define ls020DrawVertLine(x, y1 ,y2, color) ls020DrawRect(x, y1, x, y2, color)
#define ls020DrawHorizLine(x1, y, x2, color) ls020DrawRect(x1, y, x2, y, color)
#define ls020DrawPixel(x, y,color) ls020DrawRect(x, y, x, y, color)

void ls020Clear(void);

void ls020SetXY(uint8_t x, uint8_t y);
uint8_t ls020GetX(void);
void ls020LoadFont(const uint8_t *font, uint8_t color, uint8_t mult);

void ls020WriteChar(uint8_t code);
void ls020WriteString(char *string);

void ls020WriteIcon24(uint8_t iconNum);
void ls020WriteIcon32(uint8_t iconNum);

#endif /* _LCD_H */
