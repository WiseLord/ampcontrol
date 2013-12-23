#ifndef KS0108_H
#define KS0108_H

#include <inttypes.h>
#include <avr/io.h>
/* Data port */
#define GD_DPORT					PORTB
#define GD_DDDR						DDRB
#define GD_DPIN						PINB
/* Control port */
#define GD_CPORT					PORTA
#define GD_CDDR						DDRA
#define GD_CPIN						PINA

#define GD_RES						(1<<PA7)
#define GD_CS2						(1<<PA6)
#define GD_CS1						(1<<PA5)
#define GD_E						(1<<PA4)
#define GD_RW						(1<<PA3)
#define GD_DI						(1<<PA2)

#define GD_CTRL						(GD_CS1|GD_CS2|GD_E|GD_RW|GD_DI|GD_RES)

#define GD_COLS						64
#define GD_ROWS						8

/* Display control instructions */
#define KS0108_DISPLAY_ON			0x3F
#define KS0108_DISPLAY_OFF			0x3E

#define KS0108_SET_ADDRESS			0x40
#define KS0108_SET_PAGE				0xB8
#define KS0108_DISPLAY_START_LINE	0xC0

/* Status bytes */
#define STA_RESET					0x10
#define STA_DISP_OFF				0x20
#define STA_BUSY					0x80

/* Spectrum output mode */
#define MODE_STEREO					0
#define MODE_MIXED					1

void gdWriteData(uint8_t data, uint8_t cs);
void gdWriteCommand(uint8_t command, uint8_t cs);

void gdFill(uint8_t data, uint8_t cs);
void gdInit(void);

int8_t gdSetXY(uint8_t x, uint8_t y);

void gdWriteChar(unsigned char code, uint8_t inv);
void gdWriteString(uint8_t *string, uint8_t inv);
void gdWriteStringProgmem(const uint8_t *string, uint8_t inv);

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead);

void gdWriteCharScaled(uint8_t code, uint8_t scX, uint8_t scY, uint8_t inv);
void gdWriteStringScaled(uint8_t *string, uint8_t scX, uint8_t scY, uint8_t inv);
void gdWriteStringScaledProgmem(const uint8_t *string, uint8_t scX, uint8_t scY, uint8_t inv);

void gdSpectrum(uint8_t *buf, uint8_t mode);

#endif /* KS0108_H */
