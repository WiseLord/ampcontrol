#ifndef FBGD_H
#define FBGD_H

#include "st7920.h"
#include "ks0108.h"
#include "fonts.h"
#include "icons.h"

/* Display selection  */
#if !defined(ST7920) && !defined(KS0108A) && !defined(KS0108B)
#define KS0108A
#endif

enum {
	FONT_HEIGHT = 0,
	FONT_LTSPPOS,
	FONT_CCNT,
	FONT_OFTA,
	FONT_OFTNA,
	FONT_COLOR,

	FONT_END
};

enum {
	FONT_DIRECTION = FONT_END,
	FONT_FIXED,

	FONT_PARAM_END
};

enum {
	FONT_DIR_0,
	FONT_DIR_90,
	FONT_DIR_180,
	FONT_DIR_270
};

#if defined(ST7920)
#define GD_MIN_BRIGHTNESS			ST7920_MIN_BRIGHTNESS
#define GD_MAX_BRIGHTNESS			ST7920_MAX_BRIGHTNESS
#define GD_SIZE_X					ST7920_SIZE_X
#define GD_SIZE_Y					ST7920_SIZE_Y
#define	gdInit() st7920Init()
#define	gdClear() st7920Clear()
#define gdSetBrightness(br) st7920SetBrightness(br)
#define gdDrawPixel(x, y, color) st7920DrawPixel(x, y, color)
#define gdGetPins() st7920GetPins()
#else
#define GD_MIN_BRIGHTNESS			KS0108_MIN_BRIGHTNESS
#define GD_MAX_BRIGHTNESS			KS0108_MAX_BRIGHTNESS
#define GD_SIZE_X					KS0108_COLS * KS0108_CHIPS
#define GD_SIZE_Y					KS0108_ROWS * 8
#define gdInit() ks0108Init()
#define gdClear() ks0108Clear()
#define gdSetBrightness(br) ks0108SetBrightness(br)
#define gdDrawPixel(x, y, color) ks0108DrawPixel(x, y, color)
#define gdGetPins() ks0108GetPins()
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
void gdSetFontFixed(uint8_t width);

void gdWriteChar(uint8_t code);
void gdWriteString(char *string);

void gdWriteIcon24(uint8_t iconNum);
void gdWriteIcon32(uint8_t iconNum);

#endif /* FBGD_H */
