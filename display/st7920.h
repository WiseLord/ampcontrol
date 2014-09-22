#ifndef ST7920_H
#define ST7920_H

#include <avr/io.h>

#include "fonts.h"

/* #define ST7920_ENABLE_TEXTMODE_FUNCTIONS */

/* Data port */
#define ST7920_DATA_PORT			PORTB
#define ST7920_DATA_DDR				DDRB
#define ST7920_DATA_PIN				PINB

/* Control port */
#define ST7920_CTRL_PORT			PORTA
#define ST7920_CTRL_DDR				DDRA

#define ST7920_RS					(1<<PA2)
#define ST7920_RW					(1<<PA3)
#define ST7920_E					(1<<PA4)

#define ST7920_PSB					(1<<PA5)
#define ST7920_RST					(1<<PA7)

/* Basic instructions */
#define ST7920_CLEAR				0x01

#define ST7920_HOME					0x02

#define ST7920_ENTRY_MODE			0x04
	#define ST7920_INC_ADDR			0x02
	#define ST7920_SHIFT			0x01

#define ST7920_DISPLAY				0x08
	#define ST7920_DISPLAY_ON		0x04
	#define ST7920_CURSOR_ON		0x02
	#define ST7920_BLINK_ON			0x01

#define ST7920_CURSOR_CTRL			0x10
	#define ST7920_SHIFT_DISPLAY	0x08
	#define ST7920_SHIFT_RIGHT		0x04

#define ST7920_FUNCTION				0x20
	#define ST7920_8BIT				0x10
	#define ST7920_EXT_INSTR		0x04
	#define ST7920_GRAPHIC			0x02 /* Only with EXT_INSTR */

#define ST7920_SET_CGRAM			0x40

#define ST7920_SET_DDRAM			0x80

/* Extended instructions */

#define ST7920_STANDBY				0x01

#define ST7920_SCROLL_RAM			0x02
	#define ST7920_VERT_SCROLL		0x01

#define ST7920_REVERSE				0x04

#define ST7920_SET_IRAM_SCROLL		0x40

#define ST7920_SET_GRAPHIC_RAM		0x80

/* Reading display status and address */
#define ST7920_STATUS_BUSY			0x80

#define FONT_PARAM_COUNT			6
enum {
	FONT_HEIGHT,
	FONT_LTSPPOS,
	FONT_CCNT,
	FONT_OFTA,
	FONT_OFTNA,
	FONT_COLOR
};

void st7920WriteCommand(uint8_t cmd);
void st7920WriteData(uint8_t data);

void st7920Clear(void);
void st7920Init(void);

#ifdef ST7920_ENABLE_TEXTMODE_FUNCTIONS
void st7920SetTextString(uint8_t num);
void st7920WriteTextString(char *string);
#endif

void st7920LoadFont(const uint8_t *font, uint8_t color);

void st7920ReadFb(uint8_t row);
void st7920SetXY(uint8_t x, uint8_t y);
void st7920WriteFb(uint8_t row);
void st7920WriteRawToFb(uint8_t pos, uint8_t data);

void st7920WriteString(uint8_t *string);

#endif /* ST7920_H */
