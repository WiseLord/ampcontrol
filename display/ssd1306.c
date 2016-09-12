#include "ssd1306.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "../pins.h"

static uint8_t fb[SSD1306_BUFFERSIZE];

static void ssd1306Start(void)
{
	DDR(SSD1306_SCK) &= ~SSD1306_SCK_LINE;			// Pullup SCL = 1
	DDR(SSD1306_SDA) &= ~SSD1306_SDA_LINE;			// Pullup SDA = 1
	_delay_us(1);
	DDR(SSD1306_SDA) |= SSD1306_SDA_LINE;			// Active SDA = 0
	_delay_us(1);
	DDR(SSD1306_SCK) |= SSD1306_SCK_LINE;			// Active SCL = 0

	return;
}

static void ssd1306Stop(void)
{
	DDR(SSD1306_SCK) |= SSD1306_SCK_LINE;			// Active SCL = 0
	DDR(SSD1306_SDA) |= SSD1306_SDA_LINE;			// Active SDA = 0
	_delay_us(1);
	DDR(SSD1306_SCK) &= ~SSD1306_SCK_LINE;			// Pullup SCL = 1
	_delay_us(1);
	DDR(SSD1306_SDA) &= ~SSD1306_SDA_LINE;			// Pullup SDA = 1

	return;
}

static void ssd1306Write(uint8_t data)
{
	uint8_t i = 0;

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
	DDR(SSD1306_SDA) &= ~SSD1306_SDA_LINE;			// Pullup SDA = 1
	_delay_us(1);
	DDR(SSD1306_SCK) &= ~SSD1306_SCK_LINE;			// Pullup SCL = 1
	_delay_us(1);
	DDR(SSD1306_SCK) |= SSD1306_SCK_LINE;			// Active SCL = 0

	return;
}

static void ssd1306Cmd(uint8_t cmd)
{
	ssd1306Start();
	ssd1306Write(SSD1306_DEFAULT_ADDRESS);
	ssd1306Write(0x00);
	ssd1306Write(cmd);
	ssd1306Stop();
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

void ssd1306Init(void)
{
	// Turn display off
	ssd1306Cmd(SSD1306_DISPLAYOFF);

	ssd1306Cmd(SSD1306_SETDISPLAYCLOCKDIV);
	ssd1306Cmd(0x80);

	ssd1306Cmd(SSD1306_SETMULTIPLEX);
	ssd1306Cmd(0x3F);

	ssd1306Cmd(SSD1306_SETDISPLAYOFFSET);
	ssd1306Cmd(0x00);

	ssd1306Cmd(SSD1306_SETSTARTLINE | 0x00);

	// We use internal charge pump
	ssd1306Cmd(SSD1306_CHARGEPUMP);
	ssd1306Cmd(0x14);

	// Horizontal memory mode
	ssd1306Cmd(SSD1306_MEMORYMODE);
	ssd1306Cmd(0x00);

	ssd1306Cmd(SSD1306_SEGREMAP | 0x1);

	ssd1306Cmd(SSD1306_COMSCANDEC);

	ssd1306Cmd(SSD1306_SETCOMPINS);
	ssd1306Cmd(0x12);

	// Max contrast
	ssd1306Cmd(SSD1306_SETCONTRAST);
	ssd1306Cmd(0xFF);

	ssd1306Cmd(SSD1306_SETPRECHARGE);
	ssd1306Cmd(0x1F);

	ssd1306Cmd(SSD1306_SETVCOMDETECT);
	ssd1306Cmd(0x40);

	ssd1306Cmd(SSD1306_DISPLAYALLON_RESUME);

	// Non-inverted display
	ssd1306Cmd(SSD1306_NORMALDISPLAY);

	// Turn display back on
	ssd1306Cmd(SSD1306_DISPLAYON);

	// Set display D0..D7 ports as inputs with pull-up
	ssd1306SetDdrIn();

	return;
}

void ssd1306UpdateFb(void)
{
	uint16_t i;
	uint8_t *fbP = fb;

	ssd1306Start();								// 1025
	ssd1306Write(SSD1306_DEFAULT_ADDRESS);		// 1026
	ssd1306Write(0x00);							// 1027
	ssd1306Write(SSD1306_COLUMNADDR);			// 1028
	ssd1306Write(0x00);							// 1029
	ssd1306Write(0x7F);							// 1030
	ssd1306Write(SSD1306_PAGEADDR);				// 1031
	ssd1306Write(0x00);							// 1032
	ssd1306Write(0x07);							// 1033
	ssd1306Stop();								// 1034

	ssd1306Start();								// 1035
	ssd1306Write(SSD1306_DEFAULT_ADDRESS);		// 1036
	ssd1306Write(0x40);							// 1037
	for (i = 0; i < SSD1306_BUFFERSIZE; i++)	// 0..1023
		ssd1306Write(*fbP++);
	ssd1306Stop();								// 1024

	return;
}

ISR (TIMER0_OVF_vect)
{
	// 2MHz / (256 - 56) = 10000Hz
	TCNT0 = 56;

	ADCSRA |= 1<<ADSC;							// Start ADC every interrupt

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

	ssd1306Cmd(SSD1306_SETCONTRAST);
	ssd1306Cmd(rawBr);

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
