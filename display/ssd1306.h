#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_I2C_ADDR				0x78

#define SSD1306_I2C_COMMAND				0x00
#define SSD1306_I2C_DATA_SEQ			0x40

// Fundamental commands

#define SSD1306_SETLOWCOLUMN			0x00 // (Page addressing mode) 0x00..0x0F => Low nibble of column address
#define SSD1306_SETHIGHCOLUMN			0x10 // (Page addressing mode) 0x00..0x0F => High nibble of column address

#define SSD1306_MEMORYMODE				0x20 // Set memory mode, followed by 1 byte:
#define SSD1306_MEMORYMODE_HORISONTAL	0x00 // Horisontal addressing mode
#define SSD1306_MEMORYMODE_VERTICAL		0x01 // Vertical addressing mode
#define SSD1306_MEMORYMODE_PAGE			0x02 // Page addressing mode

#define SSD1306_COLUMNADDR				0x21 // (Horisontal or vertical addressing mode) // Select start and end column, followed by 2 bytes
// 0x00..0x7F => Start column
// 0x00..0x7F => End column

#define SSD1306_PAGEADDR				0x22 // (Horisontal or vertical addressing mode) // Select start and end page, followed by 2 bytes
// 0x00..0x07 => Start page
// 0x00..0x07 => End page

#define SSD1306_SETSTARTLINE			0x40 // 0x40..0x7F => Display start line

#define SSD1306_SETCONTRAST				0x81 // Set contrast, followed by 1 byte
// 0x00..0xFF => Contrast value

#define SSD1306_SEGREMAP_OFF			0xA0 // Segment remap disabled
#define SSD1306_SEGREMAP_ON				0xA1 // Segment remap enabled

#define SSD1306_ENTDISPLAY_RAM			0xA4 // Display follows GDDRAM contents
#define SSD1306_ENTDISPLAY_ON			0xA5 // Display ON regardless GDDRAM contents

#define SSD1306_NORMALDISPLAY			0xA6 // Normal display
#define SSD1306_INVERTDISPLAY			0xA7 // Inverted display

#define SSD1306_SETMULTIPLEX			0xA8 // Multiplex ratio, followed by 1 byte:
// 0x0F..0x3F => Multiplex from 0 to value lines

#define SSD1306_DISPLAY_OFF				0xAE // Display off (power saving mode)
#define SSD1306_DISPLAY_ON				0xAF // Display on

#define SSD1306_PAGE_START				0xB0 // (Page addressing mode) 0xB0..0xB7 => Selected page

#define SSD1306_COMSCANINC				0xC0 // Scan from COM[0] to COM[Multiplex]
#define SSD1306_COMSCANDEC				0xC8 // Scan from COM[Multiplex] to COM[0]

#define SSD1306_SETDISPLAYOFFSET		0xD3 // Display offset, followed by 1 byte (0x00..0x3F)

#define SSD1306_SETDISPLAYCLOCKDIV		0xD5 // Clock divider and frequency, followed by 1 byte:
// 0bDDDDFFFF

#define SSD1306_SETPRECHARGE			0xD9 // Pre-charge period, phase A and B,followed by 1 byte:
// 0bAAAABBBB

#define SSD1306_SETCOMPINS				0xDA // COM pins hardware configuration, followed by 1 byte:
// 0b00AB0010, A = 0: Disable left/right remap, 1: Enable left/right remap; B = 0: Sequential, 1: Alternative

#define SSD1306_SETVCOMDETECT			0xDB // Vcomh regulator output, followed by 1 byte:
// 0b0VVV0000

#define SSD1306_NOP						0xE3 // No operation command

// Graphic acceleration commands

#define SSD1306_SCROLL_HORIZ_RIGHT		0x26 // Right horizontal scroll, or
#define SSD1306_SCROLL_HORIZ_LEFT		0x27 // Left horizontal scroll, followed by 6 bytes:
// 0x00: dummy byte
// 0x00..0x07: start page address
// 0x00..0x07: scroll step inteval (5, 64, 128, 256, 3, 4, 25, 2), frames
// 0x00..0x07: end page address
// 0x00: dymmy byte
// 0xFF: dymmy byte

#define SSD1306_SCROLL_VERTHORIZ_RIGHT	0x29 // Vertical and right horizontal scroll, or
#define SSD1306_SCROLL_VERTHORIZ_LEFT	0x2A // Vertical and left horizontal scroll, followed by 5 bytes:
// 0x00: dummy byte
// 0x00..0x07: start page addres
// 0x00..0x07: scroll step inteval (5, 64, 128, 256, 3, 4, 25, 2), frames
// 0x00..0x07: end page address
// 0x00..0x3F: vertical scrolling offset (0..63 rows)

#define SSD1306_SCROLL_DEACTIVATE		0x2E // Deactivate scroll
#define SSD1306_SCROLL_ACTIVATE			0x2F // Activate scroll

#define SSD1306_SCROLL_AREA				0xA3 // Set scroll area, followed by 2 bytes:
// 0x00..0x3F: number of rows in top fixed area
// 0x00..0x7F: number of rows in scroll area

// Charge pump setup commands

#define SSD1306_CHARGEPUMP				0x8D // Select charge pump, followed by 1 byte:
//0bXX010A00, A = 0: Disable (default), 1: Enable during display on

// Other definitions

#define SSD1306_WIDTH					128
#define SSD1306_HEIGHT					64
#define SSD1306_BUFFERSIZE				(SSD1306_WIDTH * SSD1306_HEIGHT / 8)

#define SSD1306_MIN_BRIGHTNESS			0
#define SSD1306_MAX_BRIGHTNESS			32

uint8_t ssd1306GetPins(void);

void ssd1306UpdateFb(void);

void ssd1306Init(void);
void ssd1306DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void ssd1306Clear(void);

void ssd1306SetBrightness(uint8_t br);

#endif // SSD1306_H
