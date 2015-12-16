#ifndef KS0108_H
#define KS0108_H

#include <avr/io.h>
#include <inttypes.h>

#include "fonts.h"

/* Data port */
#define KS0108_DATA_PORT			PORTB
#define KS0108_DATA_DDR				DDRB
#define KS0108_DATA_PIN				PINB

/* Control port */
#define KS0108_CTRL_PORT			PORTA
#define KS0108_CTRL_DDR				DDRA

#define KS0108_E					(1<<PA4)
#define KS0108_RW					(1<<PA3)
#define KS0108_DI					(1<<PA2)

/* Chip select/reset port */
#define KS0108_CHIP_PORT			PORTA
#define KS0108_CHIP_DDR				DDRA

#define KS0108_RES					(1<<PA7)
#define KS0108_CS2					(1<<PA6)
#define KS0108_CS1					(1<<PA5)

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

#define CS_ENABLE					1
#define CS_DISABLE					0

#define FONT_PARAM_COUNT			6

#define KS0108_SET_CS1(); \
	KS0108_CHIP_PORT |= KS0108_CS1; \
	KS0108_CHIP_PORT &= ~KS0108_CS2;

#define KS0108_SET_CS2(x); \
	KS0108_CHIP_PORT |= KS0108_CS2; \
	KS0108_CHIP_PORT &= ~KS0108_CS1;

enum {
	FONT_HEIGHT,
	FONT_LTSPPOS,
	FONT_CCNT,
	FONT_OFTA,
	FONT_OFTNA,
	FONT_COLOR
};

void ks0108WriteCommand(uint8_t command);
void ks0108WriteData(uint8_t data);

void ks0108Clear(void);
void ks0108Init(void);

void ks0108SetXY(uint8_t x, uint8_t y);

void ks0108LoadFont(const uint8_t *font, uint8_t color);

void ks0108WriteChar(uint8_t code);
void ks0108WriteString(uint8_t *string);

#endif /* KS0108_H */
