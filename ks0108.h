#ifndef KS0108_H
#define KS0108_H

#include <avr/io.h>
#include <inttypes.h>

#include "fonts.h"

/*
 * Some displays like WG12864B have inverted (zero) active level on CS1/CS2.
 * Set CS_INVERTED for such displays.
 */
/*#define CS_INVERTED */

/* Data port */
#define GD_DATA_PORT				PORTB
#define GD_DATA_DDR					DDRB
#define GD_DATA_PIN					PINB

/* Control port */
#define GD_CONTROL_PORT				PORTA
#define GD_CONTROL_DDR				DDRA

#define GD_E						(1<<PA4)
#define GD_RW						(1<<PA3)
#define GD_DI						(1<<PA2)

/* Chip select/reset port */
#define GD_CHIP_PORT				PORTA
#define GD_CHIP_DDR					DDRA

#define GD_RES						(1<<PA7)
#define GD_CS2						(1<<PA6)
#define GD_CS1						(1<<PA5)

/* Backlight port */
#define GD_BACKLIGHT_DDR			DDRC
#define GD_BACKLIGHT_PORT			PORTC

#define GD_BCKL						(1<<PC7)

/* Display geometry */
#define GD_COLS						64
#define GD_ROWS						8
#define GD_CHIPS					2

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

/* Spectrum output mode */
#define MODE_STEREO					0
#define MODE_MIXED					1

typedef struct {
	uint8_t height;
	uint8_t ltsppos;
	uint8_t ccnt;
	uint8_t ofta;
	uint8_t oftna;
	uint8_t color;
} fontParams;

uint8_t gdReadStatus();
uint8_t gdReadData();

void gdWriteCommand(uint8_t command);
void gdWriteData(uint8_t data);

void gdFill(uint8_t data);
void gdInit(void);

void gdSetXY(uint8_t x, uint8_t y);

void gdLoadFont(const uint8_t *font, uint8_t color);

void gdWriteChar(uint8_t code);
void gdWriteString(uint8_t *string);
void gdWriteStringProgmem(const uint8_t *string);
void gdWriteStringEeprom(const uint8_t *string);

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead, uint8_t radix);

void gdSpectrum32(uint8_t *buf, uint8_t mode);

#endif /* KS0108_H */
