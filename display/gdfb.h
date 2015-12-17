#ifndef FBGD_H
#define FBGD_H

/* Display selection  */
#if !defined(ST7920) && !defined(KS0108A) && !defined(KS0108B)
#define KS0108A
#endif

#if defined(ST7920)

#include "st7920.h"

#define GD_MIN_BRIGHTNESS			ST7920_MIN_BRIGHTNESS
#define GD_MAX_BRIGHTNESS			ST7920_MAX_BRIGHTNESS

#define GD_SIZE_X					ST7920_SIZE_X
#define GD_SIZE_Y					ST7920_SIZE_Y

#else

#include "ks0108.h"

#define GD_MIN_BRIGHTNESS			KS0108_MIN_BRIGHTNESS
#define GD_MAX_BRIGHTNESS			KS0108_MAX_BRIGHTNESS

#define GD_SIZE_X					KS0108_COLS * KS0108_CHIPS
#define GD_SIZE_Y					KS0108_ROWS * 8

#endif

#include "fonts.h"
#include "icons.h"

#define FONT_PARAM_COUNT			7
enum {
	FONT_HEIGHT,
	FONT_LTSPPOS,
	FONT_CCNT,
	FONT_OFTA,
	FONT_OFTNA,
	FONT_COLOR,
	FONT_DIRECTION
};

enum {
	FONT_DIR_0,
	FONT_DIR_90,
	FONT_DIR_180,
	FONT_DIR_270
};

#if defined(ST7920)
#define	gdInit() st7920Init()
#define	gdClear() st7920Clear()
#define gdSetBrightness(br) st7920SetBrightness(br)
#define gdDrawPixel(x, y, color) st7920DrawPixel(x, y, color)
#else
#define gdInit() ks0108Init()
#define gdClear() ks0108Clear()
#define gdSetBrightness(br) ks0108SetBrightness(br);
#define gdDrawPixel(x, y, color) ks0108DrawPixel(x, y, color)
#endif

void gdDrawHorizLine(uint8_t x1, uint8_t x2, uint8_t y, uint8_t color);
void gdDrawVertLine(uint8_t x, uint8_t y1, uint8_t y2, uint8_t color);
void gdDrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

void gdDrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void gdDrawFilledRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

void gdDrawCircle(uint8_t x0, uint8_t y0, int16_t radius, uint8_t color);

void gdSetXY(uint8_t x, uint8_t y);
uint8_t gdGetX(void);
void gdLoadFont(const uint8_t *font, uint8_t color, uint8_t direction);

void gdWriteChar(uint8_t code);
void gdWriteString(char *string);

void gdWriteIcon24(uint8_t iconNum);
void gdWriteIcon32(uint8_t iconNum);

#endif /* FBGD_H */
