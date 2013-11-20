#ifndef KS0108_H
#define KS0108_H

#include <inttypes.h>
#include <avr/io.h>

#define GD_DPORT					PORTB
#define GD_DDDR						DDRB
#define GD_DPIN						PINB

#define GD_CPORT					PORTC
#define GD_CDDR						DDRC
#define GD_CPIN						PINC

#define RES							(1<<PC2)
#define CS2							(1<<PC3)
#define CS1							(1<<PC4)
#define E							(1<<PC5)
#define RW							(1<<PC6)
#define DI							(1<<PC7)

#define GD_COLS						64
#define GD_ROWS						8

#define GD_COMM						0
#define GD_DATA						1

// Display control instructions
#define KS0108_DISPLAY_ON			0x3F
#define KS0108_DISPLAY_OFF			0x3E

#define KS0108_SET_ADDRESS			0x40
#define KS0108_SET_PAGE				0xB8
#define KS0108_DISPLAY_START_LINE	0xC0

// Status bytes
#define STA_RESET					0x10
#define STA_DISP_OFF				0x20
#define STA_BUSY					0x80

// Spectrum output mode
#define MODE_MIXED					0
#define MODE_LEFT					1
#define MODE_RIGHT					2
#define MODE_STEREO					3

void gdWrite(uint8_t mode, uint8_t data, uint8_t cs);
void gdInit(void);
void gdFill(uint8_t data, uint8_t cs);
void gdSpectrum(uint8_t *buf, uint8_t mode);

void gdSetPos(uint8_t x, uint8_t y);
void gdWriteChar(unsigned char code);
void gdWriteString(uint8_t *string);
void gdWriteNum(int16_t number, uint8_t width);

void gdWriteChar2(uint8_t code, uint8_t line);
void gdWriteString2(uint8_t x, uint8_t y, uint8_t *string);

#endif // KS0108_H
