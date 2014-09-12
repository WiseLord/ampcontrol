#ifndef FBGD_H
#define FBGD_H

/* Display selection  */
#if !defined(ST7920) && !defined(KS0108A) && !defined(KS0108B)
#define KS0108A
#endif

#if defined(ST7920)
#include "st7920.h"
#elif defined(KS0108A) || defined(KS0108B)
#include "ks0108.h"
#endif
#include "fonts.h"

/* Backlight port */
#define DISPLAY_BCKL_DDR			DDRC
#define DISPLAY_BCKL_PORT			PORTC
#define DISPLAY_BCKL				(1<<PC7)

#define GD_SIZE_X					128
#define GD_SIZE_Y					64

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

inline void gdInit(void);
inline void gdClear(void);
inline void gdDrawPixel(uint8_t x, uint8_t y, uint8_t color);

void gdDrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
void gdDrawCircle(uint8_t x0, uint8_t y0, int16_t radius, uint8_t color);

void gdLoadFont(const uint8_t *font, uint8_t color, uint8_t direction);
void gdSetXY(uint8_t x, uint8_t y);
void gdWriteChar(uint8_t code);
void gdWriteString(uint8_t *string);

#endif /* FBGD_H */
