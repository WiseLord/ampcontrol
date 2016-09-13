#include "ssd1306.h"

#include "../pins.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

static uint8_t fb[SSD1306_BUFFERSIZE];

static const uint8_t initSeq[] PROGMEM = {
	SSD1306_DISPLAY_OFF,
	SSD1306_SETDISPLAYCLOCKDIV,
	0xF0,
	SSD1306_SETMULTIPLEX,
	0x3F,
	SSD1306_SETDISPLAYOFFSET,
	0x00,
	SSD1306_SETSTARTLINE | 0x00,
	SSD1306_MEMORYMODE,
	SSD1306_MEMORYMODE_HORISONTAL,
	SSD1306_SEGREMAP_ON,
	SSD1306_COMSCANDEC,
	SSD1306_SETCOMPINS,
	0x12,
	SSD1306_SETCONTRAST,
	0xFF,
	SSD1306_SETPRECHARGE,
	0x1F,
	SSD1306_SETVCOMDETECT,
	0x40,
	SSD1306_ENTDISPLAY_RAM,
	SSD1306_NORMALDISPLAY,
	SSD1306_CHARGEPUMP,
	0x14,
	SSD1306_DISPLAY_ON,
};

static const uint8_t dispAreaSeq[] PROGMEM = {
	SSD1306_COLUMNADDR,
	0x00,
	0x7F,
	SSD1306_PAGEADDR,
	0x00,
	0x07,
};

static void _I2CWriteByte(uint8_t data)
{
	uint8_t i = 0;

	// Data bits
	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			DDR(SSD1306_SDA) &= ~SSD1306_SDA_LINE;	// Pullup SDA = 1
		else
			DDR(SSD1306_SDA) |= SSD1306_SDA_LINE;	// Active SDA = 0
		_delay_us(1);
		DDR(SSD1306_SCK) &= ~SSD1306_SCK_LINE;		// Pullup SCL = 1
		_delay_us(1);
		DDR(SSD1306_SCK) |= SSD1306_SCK_LINE;		// Active SCL = 0
		data <<= 1;
	}
	// ACK bit
	DDR(SSD1306_SDA) &= ~SSD1306_SDA_LINE;			// Pullup SDA = 1
	_delay_us(1);
	DDR(SSD1306_SCK) &= ~SSD1306_SCK_LINE;			// Pullup SCL = 1
	_delay_us(1);
	DDR(SSD1306_SCK) |= SSD1306_SCK_LINE;			// Active SCL = 0

	return;
}

static void _I2CStart(uint8_t addr)
{
	DDR(SSD1306_SCK) &= ~SSD1306_SCK_LINE;			// Pullup SCL = 1
	DDR(SSD1306_SDA) &= ~SSD1306_SDA_LINE;			// Pullup SDA = 1
	_delay_us(1);
	DDR(SSD1306_SDA) |= SSD1306_SDA_LINE;			// Active SDA = 0
	_delay_us(1);
	DDR(SSD1306_SCK) |= SSD1306_SCK_LINE;			// Active SCL = 0

	_I2CWriteByte(addr);

	return;
}

static void _I2CStop(void)
{
	DDR(SSD1306_SCK) |= SSD1306_SCK_LINE;			// Active SCL = 0
	DDR(SSD1306_SDA) |= SSD1306_SDA_LINE;			// Active SDA = 0
	_delay_us(1);
	DDR(SSD1306_SCK) &= ~SSD1306_SCK_LINE;			// Pullup SCL = 1
	_delay_us(1);
	DDR(SSD1306_SDA) &= ~SSD1306_SDA_LINE;			// Pullup SDA = 1

	return;
}

static void ssd1306SendCmd(uint8_t cmd)
{
	_I2CWriteByte(cmd);

	return;
}

static void ssd1306SetDdrIn(void)
{
	// Set ports as inputs
	DDR(DISP_D0) &= ~DISP_D0_LINE;
	DDR(DISP_D1) &= ~DISP_D1_LINE;
	DDR(DISP_D2) &= ~DISP_D2_LINE;
	DDR(DISP_D3) &= ~DISP_D3_LINE;
	DDR(DISP_D4) &= ~DISP_D4_LINE;
	DDR(DISP_D5) &= ~DISP_D5_LINE;
	DDR(DISP_D6) &= ~DISP_D6_LINE;
	DDR(DISP_D7) &= ~DISP_D7_LINE;
	// Add pullup resistors
	PORT(DISP_D0) |= DISP_D0_LINE;
	PORT(DISP_D1) |= DISP_D1_LINE;
	PORT(DISP_D2) |= DISP_D2_LINE;
	PORT(DISP_D3) |= DISP_D3_LINE;
	PORT(DISP_D4) |= DISP_D4_LINE;
	PORT(DISP_D5) |= DISP_D5_LINE;
	PORT(DISP_D6) |= DISP_D6_LINE;
	PORT(DISP_D7) |= DISP_D7_LINE;

	return;
}

uint8_t ssd1306GetPins(void)
{
	uint8_t ret = 0x00;

	if (PIN(DISP_D0) & DISP_D0_LINE) ret |= (1<<0);
	if (PIN(DISP_D1) & DISP_D1_LINE) ret |= (1<<1);
	if (PIN(DISP_D2) & DISP_D2_LINE) ret |= (1<<2);
	if (PIN(DISP_D3) & DISP_D3_LINE) ret |= (1<<3);
	if (PIN(DISP_D4) & DISP_D4_LINE) ret |= (1<<4);
	if (PIN(DISP_D5) & DISP_D5_LINE) ret |= (1<<5);
	if (PIN(DISP_D6) & DISP_D6_LINE) ret |= (1<<6);
	if (PIN(DISP_D7) & DISP_D7_LINE) ret |= (1<<7);

	return ~ret;
}

void ssd1306Init(void)
{
	uint8_t i;

	_I2CStart(SSD1306_I2C_ADDR);
	_I2CWriteByte(SSD1306_I2C_COMMAND);

	for (i = 0; i < sizeof(initSeq); i++)
		ssd1306SendCmd(pgm_read_byte(&initSeq[i]));

	_I2CStop();

	// Set display D0..D7 ports as inputs with pull-up
	ssd1306SetDdrIn();

	return;
}

ISR (TIMER0_OVF_vect)
{
	// 2MHz / (256 - 56) = 10000Hz
	TCNT0 = 56;

	ADCSRA |= 1<<ADSC;							// Start ADC every interrupt

	return;
}

void ssd1306UpdateFb(void)
{
	uint16_t i;
	uint8_t *fbP = fb;

	_I2CStart(SSD1306_I2C_ADDR);
	_I2CWriteByte(SSD1306_I2C_COMMAND);

	for (i = 0; i < sizeof(dispAreaSeq); i++)
		ssd1306SendCmd(pgm_read_byte(&dispAreaSeq[i]));

	_I2CStop();

	_I2CStart(SSD1306_I2C_ADDR);
	_I2CWriteByte(SSD1306_I2C_DATA_SEQ);

	for (i = 0; i < SSD1306_BUFFERSIZE; i++)
		_I2CWriteByte(*fbP++);

	_I2CStop();

	return;
}

void ssd1306DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
	uint8_t bit;
	uint8_t *fbP;

	if (x >= 128)
		return;
	if (y >= 64)
		return;

	fbP = &fb[(y >> 3) * 128 + x];

	bit = 1 << (y & 0x07);

	if (color)
		*fbP |= bit;
	else
		*fbP &= ~bit;

	return;
}

void ssd1306Clear(void)
{
	uint16_t i;
	uint8_t *fbP = fb;

	for (i = 0; i < SSD1306_BUFFERSIZE; i++)
		*fbP++ = 0x00;

	return;
}

void ssd1306SetBrightness(uint8_t br)
{
	uint8_t rawBr = 255;

	if (br < SSD1306_MAX_BRIGHTNESS)
		rawBr = br * 8;

	_I2CStart(SSD1306_I2C_ADDR);
	_I2CWriteByte(SSD1306_I2C_COMMAND);

	ssd1306SendCmd(SSD1306_SETCONTRAST);
	ssd1306SendCmd(rawBr);

	_I2CStop();

	return;
}
