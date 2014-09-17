#ifndef KS0108_H
#define KS0108_H

#include <avr/io.h>

#ifdef KS0108B
#define KS0108_CS_INV
#endif

/* Data port */
#define KS0108_DATA_PORT			PORTB
#define KS0108_DATA_DDR				DDRB
#define KS0108_DATA_PIN				PINB

/* Control port */
#define KS0108_CTRL_PORT			PORTA
#define KS0108_CTRL_DDR				DDRA

#define KS0108_DI					(1<<PA2)
#define KS0108_RW					(1<<PA3)
#define KS0108_E					(1<<PA4)
#define KS0108_CS1					(1<<PA5)
#define KS0108_CS2					(1<<PA6)
#define KS0108_RES					(1<<PA7)

/* Backlight port */
#define KS0108_BCKL_DDR				DDRC
#define KS0108_BCKL_PORT			PORTC
#define KS0108_BCKL					(1<<PC7)

#define KS0108_CS_ALL				(KS0108_CS1 | KS0108_CS2)

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

uint8_t isAdcResultReady(void);

void ks0108Init(void);
void ks0108Clear();

void ks0108DrawPixel(uint8_t x, uint8_t y, uint8_t color);

#endif /* KS0108_H */
