#ifndef FBGD_H
#define FBGD_H

#include "st7920.h"
#include "fonts.h"

#define FONT_PARAM_COUNT			6
enum {
	FONT_HEIGHT,
	FONT_LTSPPOS,
	FONT_CCNT,
	FONT_OFTA,
	FONT_OFTNA,
	FONT_COLOR
};

#define gdDrawPixel(x, y, color) st7920DrawPixel(x, y, color);

void gdDrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
void gdDrawCircle(uint8_t x0, uint8_t y0, int16_t radius, uint8_t color);

void gdLoadFont(const uint8_t *font, uint8_t color);
void gdSetXY(uint8_t x, uint8_t y);
void gdWriteChar(uint8_t code);
void gdWriteString(uint8_t *string);

#endif /* FBGD_H */
