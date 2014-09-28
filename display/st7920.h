#ifndef ST7920_H
#define ST7920_H

#include <avr/io.h>

/* Data port */
#define ST7920_DATA_PORT			PORTB
#define ST7920_DATA_DDR				DDRB
#define ST7920_DATA_PIN				PINB

/* Control port */
#define ST7920_CTRL_PORT			PORTA
#define ST7920_CTRL_DDR				DDRA

/* Backlight port */
#define ST7920_BCKL_DDR				DDRC
#define ST7920_BCKL_PORT			PORTC
#define ST7920_BCKL					(1<<PC7)

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

/* Some common defines */
#define ST7920_COMMAND				0
#define ST7920_DATA					1

#define ST7920_SIZE_X				128
#define ST7920_SIZE_Y				64

#define ST7920_MIN_BRIGHTNESS		0
#define ST7920_MAX_BRIGTHNESS		32

void setAdcTimer(uint8_t value);

void st7920SetBrightness(uint8_t br);

void st7920Init(void);
void st7920Clear();

void st7920DrawPixel(uint8_t x, uint8_t y, uint8_t color);

#endif /* ST7920_H */
