#ifndef KS0108_H
#define KS0108_H

#include <inttypes.h>
#include "../pins.h"

#ifdef KS0108B
#define KS0108_CS_INV
#endif

/* Display geometry */
#define KS0108_COLS					64
#define KS0108_ROWS					8
#define KS0108_CHIPS				2

/* Display control instructions */
#define KS0108_DISPLAY_ON			0x3F
#define KS0108_DISPLAY_OFF			0x3E

#define KS0108_SET_ADDRESS			0x40
#define KS0108_SET_PAGE				0xB8
#define KS0108_DISPLAY_START_LINE	0xC0

/* Status bytes */
#define KS0108_STA_RESET			0x10	/* DB7 */
#define KS0108_STA_DISP_OFF			0x20	/* DB5 */
#define KS0108_STA_BUSY				0x80	/* DB4 */

/* Some common defines */
#define KS0108_COMMAND				0
#define KS0108_DATA					1

#define KS0108_MIN_BRIGHTNESS		0
#define KS0108_MAX_BRIGTHNESS		32

void ks0108SetBrightness(uint8_t br);

void ks0108Init(void);
void ks0108Clear();

void ks0108DrawPixel(uint8_t x, uint8_t y, uint8_t color);

#endif /* KS0108_H */
