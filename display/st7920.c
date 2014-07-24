#include "st7920.h"

#include "util/delay.h"

#include <avr/pgmspace.h>

const uint8_t *_font;
static uint8_t fp[FONT_PARAM_COUNT];

static uint8_t fb[128];
static uint8_t _col, _row;

static uint8_t st7920Strob()
{
	uint8_t pin;

	ST7920_CTRL_PORT |= ST7920_E;
	_delay_us(0.32);
	pin = ST7920_DATA_PIN;
	ST7920_CTRL_PORT &= ~ST7920_E;

	return pin;
}

static uint8_t st7920ReadStatus()
{
	uint8_t status;

	ST7920_DATA_DDR = 0x00;

	ST7920_CTRL_PORT |= ST7920_RW;
	ST7920_CTRL_PORT &= ~ST7920_RS;

	status = st7920Strob();

	return status;
}

static void st7920WaitWhile(uint8_t status)
{
	uint8_t i = 0;

	while (st7920ReadStatus() & status) {
		if (i++ > 200)	/* Avoid endless loop */
			return;
	}

	return;
}

void st7920WriteCommand(uint8_t cmd)
{
	st7920WaitWhile(ST7920_STATUS_BUSY);

	ST7920_DATA_DDR = 0xFF;

	ST7920_CTRL_PORT &= ~ST7920_RS;
	ST7920_CTRL_PORT &= ~ST7920_RW;

	ST7920_DATA_PORT = cmd;

	st7920Strob();

	return;
}

void st7920WriteData(uint8_t data)
{
	st7920WaitWhile(ST7920_STATUS_BUSY);

	ST7920_DATA_DDR = 0xFF;

	ST7920_CTRL_PORT |= ST7920_RS;
	ST7920_CTRL_PORT &= ~ST7920_RW;

	ST7920_DATA_PORT = data;

	st7920Strob();

	return;
}

static uint8_t st7920ReadData()
{
	uint8_t data;

	st7920WaitWhile(ST7920_STATUS_BUSY);
	ST7920_CTRL_PORT |= ST7920_RS;

	data = st7920Strob();

	return data;
}

void st7920Fill(uint8_t data)
{
	uint8_t x, y;

	for(y = 0; y < 64; y++) {
		if (y < 32) {
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | y);
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM);
		} else {
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | (y - 32));
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | 0x08);
		}
		for(x = 0; x < 8; x++) {
			st7920WriteData(data);
			st7920WriteData(data);
		}
	}

	return;
}

void st7920Init(void)
{
	/* Set control lines as outputs */
	ST7920_CTRL_DDR |= (ST7920_RS | ST7920_RW | ST7920_E | ST7920_PSB | ST7920_RST);
	ST7920_DATA_DDR = 0xFF;

	ST7920_CTRL_PORT &= ~(ST7920_RS | ST7920_RW | ST7920_E);
	/* Switch display to parallel mode */
	ST7920_CTRL_PORT |= ST7920_PSB;

	/* Reset display */
	ST7920_CTRL_PORT &= ~ST7920_RST;
	_delay_us(1);
	ST7920_CTRL_PORT |= ST7920_RST;
	_delay_ms(40);

	/* Init display */
	st7920WriteCommand(ST7920_FUNCTION | ST7920_8BIT);
	_delay_us(100);
	st7920WriteCommand(ST7920_FUNCTION | ST7920_8BIT);
	_delay_us(40);
	st7920WriteCommand(ST7920_DISPLAY | ST7920_DISPLAY_ON);
	_delay_us(100);
	st7920WriteCommand(ST7920_CLEAR);
	_delay_ms(10);
	st7920WriteCommand(ST7920_ENTRY_MODE | ST7920_INC_ADDR);

	/* Enable graphic mode */
	st7920WriteCommand(ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR);
	st7920WriteCommand(ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR | ST7920_GRAPHIC);

	return;
}

void st7920LoadFont(const uint8_t *font, uint8_t color)
{
	uint8_t i;

	_font = font + 5;
	for (i = 0; i < FONT_PARAM_COUNT - 1; i++)
		fp[i] = pgm_read_byte(font + i);
	fp[FONT_COLOR] = color;

	return;
}

#ifdef ST7920_ENABLE_TEXTMODE_FUNCTIONS
void st7920SetTextString(uint8_t num)
{
	switch (num) {
	case 1:
		st7920WriteCommand(ST7920_SET_DDRAM | 0x10); /* String 1 */
		break;
	case 2:
		st7920WriteCommand(ST7920_SET_DDRAM | 0x08); /* String 2 */
		break;
	case 3:
		st7920WriteCommand(ST7920_SET_DDRAM | 0x18); /* String 3 */
		break;
	default:
		st7920WriteCommand(ST7920_SET_DDRAM | 0x00); /* String 0 */
		break;
	}

	return;
}

void st7920WriteTextString(char *string)
{
	while(*string)
		st7920WriteData(*string++);

	return;
}
#endif

static void st7920WriteFb(uint8_t row)
{
	uint8_t i, j, k;
	uint8_t data;

	for (i = 0; i < 8; i++) {
		if (row < 4) {
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | (row * 8 + i));
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM);
		} else {
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | (row * 8 - 32 + i));
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | 0x08);
		}

		for (j = 0; j < 16; j++) {
			data = 0x00;
			for (k = 0; k < 8; k++) {
				if (fb[j * 8 + k] & (1<<i))
					data |= (1<<(7 - k));
			}

			st7920WriteData(data);
		}
	}
}

static void st7920ReadFb(uint8_t row)
{
	uint8_t i, j, k;
	uint8_t data;

	for (i = 0; i < 8; i++) {
		if (row < 4) {
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | (row * 8) | i);
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM);
		} else {
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | (row * 8 - 32) | i);
			st7920WriteCommand(ST7920_SET_GRAPHIC_RAM | 0x08);
		}

		st7920ReadData();		/* Dubby read after setting address */

		for (j = 0; j < 16; j++) {
			data = st7920ReadData();
			for (k = 0; k < 8; k++) {
				if (data & (1<<(7 - k)))
					fb[j * 8 + k] |= (1<<i);
				else
					fb[j * 8 + k] &= ~(1<<i);
			}
		}
	}
}

void st7920SetXY(uint8_t x, uint8_t y)
{
	_col = x;
	_row = y;

	st7920ReadFb(_row);

	return;
}

static void st7920WriteChar(uint8_t code, uint8_t part)
{
	uint8_t i;

	uint8_t pgmData;

	uint8_t spos = code - ((code >= 128) ? fp[FONT_OFTNA] : fp[FONT_OFTA]);

	uint16_t oft = 0;	/* Current symbol offset in array*/
	uint8_t swd = 0;	/* Current symbol width */

	for (i = 0; i < spos; i++) {
		swd = pgm_read_byte(_font + i);
		oft += swd;
	}
	swd = pgm_read_byte(_font + spos);

	oft *= fp[FONT_HEIGHT];
	oft += fp[FONT_CCNT];

	for (i = 0; i < swd; i++) {
		pgmData = pgm_read_byte(_font + oft + (swd * part) + i);
		if (fp[FONT_COLOR])
			fb[_col++] = pgmData;
		else
			fb[_col++] = ~pgmData;
	}

}

void st7920WriteString(uint8_t *string)
{
	uint8_t i;
	uint8_t *str;

	uint8_t col = _col;
	uint8_t row = _row;

	for (i = 0; i < fp[FONT_HEIGHT]; i++) {
		st7920SetXY(col, row + i);
		str = string;
		if (*str)
			st7920WriteChar(*str++, i);
		while(*str) {
			st7920WriteChar(fp[FONT_LTSPPOS], i);
			st7920WriteChar(*str++, i);
		}
		st7920WriteFb(_row);
	}
	_row = row;

	return;
}
