#include "ks0108.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

const uint8_t *_font;
static uint8_t _row, _col;

static uint8_t fp[FONT_PARAM_COUNT];

static void ks0108Strob()
{
    KS0108_CTRL_PORT |= KS0108_E;
    asm("nop");
    KS0108_CTRL_PORT &= ~KS0108_E;

    return;
}

void ks0108WriteCommand(uint8_t command)
{
    _delay_us(50);
    KS0108_CTRL_PORT &= ~KS0108_DI;
    KS0108_DATA_PORT = command;
    ks0108Strob();

    return;
}

void ks0108WriteData(uint8_t data)
{
    _delay_us(50);
    KS0108_CTRL_PORT |= KS0108_DI;
    KS0108_DATA_PORT = data;
    ks0108Strob();

    if (++_col >= KS0108_COLS * KS0108_CHIPS) {
        _col = 0;
        if (++_row >= KS0108_ROWS)
            _row = 0;
    }

    if (_col == KS0108_COLS || _col == 0) {

        if (_col == KS0108_COLS) {
            KS0108_SET_CS2();
        } else if (_col == 0) {
            KS0108_SET_CS1();
        }

        ks0108WriteCommand(KS0108_SET_ADDRESS + (_col & (KS0108_COLS - 1)));
        ks0108WriteCommand(KS0108_SET_PAGE + _row);
    }
    return;
}

void ks0108Clear(void)
{
    uint8_t i, j;

    ks0108SetXY(0, 0);

    for (i = 0; i < KS0108_ROWS; i++)
        for (j = 0; j < KS0108_COLS * KS0108_CHIPS; j++)
            ks0108WriteData(0x00);

    return;
}

void ks0108Init(void)
{
    // Set control lines as outputs
    KS0108_CTRL_DDR |= KS0108_DI | KS0108_RW | KS0108_E;
    KS0108_CHIP_DDR |= KS0108_CS1 | KS0108_CS2 | KS0108_RES;
    KS0108_DATA_DDR = 0xFF;

    // Reset
    KS0108_CHIP_PORT &= ~(KS0108_RES);
    asm("nop");
    asm("nop");
    KS0108_CHIP_PORT |= KS0108_RES;
    asm("nop");
    asm("nop");

    // Always in write mode
    KS0108_CTRL_PORT &= ~KS0108_RW;

    // Init first controller
    KS0108_SET_CS1();
    ks0108WriteCommand(KS0108_DISPLAY_START_LINE);
    ks0108WriteCommand(KS0108_DISPLAY_ON);
    // Init second controller
    KS0108_SET_CS2();
    ks0108WriteCommand(KS0108_DISPLAY_START_LINE);
    ks0108WriteCommand(KS0108_DISPLAY_ON);

    fp[FONT_HEIGHT] = 1;
    ks0108Clear();

    _row = 0;
    _col = 0;

    return;
}

void ks0108SetXY(uint8_t x, uint8_t y)
{
    _col = x;
    _row = y;

    if (_col >= KS0108_COLS * KS0108_CHIPS)
        _col = 0;
    if (_row >= KS0108_ROWS)
        _row = 0;

    if (_col >= KS0108_COLS) {
        KS0108_SET_CS2();
    } else {
        KS0108_SET_CS1();
    }

    ks0108WriteCommand(KS0108_SET_ADDRESS + (_col & (KS0108_COLS - 1)));
    ks0108WriteCommand(KS0108_SET_PAGE + _row);

    return;
}

void ks0108LoadFont(const uint8_t *font, uint8_t color)
{
    uint8_t i;

    _font = font + 5;
    for (i = 0; i < FONT_PARAM_COUNT - 1; i++)
        fp[i] = pgm_read_byte(font + i);
    fp[FONT_COLOR] = color;
}

void ks0108WriteChar(uint8_t code)
{
    // Store current position before writing to display
    uint8_t row = _row;
    uint8_t col = _col;

    uint8_t i, j;

    uint8_t pgmData;

    uint8_t spos = code - ((code >= 128) ? fp[FONT_OFTNA] : fp[FONT_OFTA]);

    uint16_t oft = 0;   // Current symbol offset in array
    uint8_t swd = 0;    // Current symbol width

    for (i = 0; i < spos; i++) {
        swd = pgm_read_byte(_font + i);
        oft += swd;
    }
    swd = pgm_read_byte(_font + spos);

    oft *= fp[FONT_HEIGHT];
    oft += fp[FONT_CCNT];

    for (j = 0; j < fp[FONT_HEIGHT]; j++) {
        ks0108SetXY(col, row + j);
        for (i = 0; i < swd; i++) {
            pgmData = pgm_read_byte(_font + oft + (swd * j) + i);
            if (fp[FONT_COLOR])
                ks0108WriteData(pgmData);
            else
                ks0108WriteData(~pgmData);
        }
    }

    ks0108SetXY(col + swd, row);

    return;
}

void ks0108WriteString(char *string)
{
    if (*string)
        ks0108WriteChar(*string++);
    while (*string) {
        ks0108WriteChar(fp[FONT_LTSPPOS]);
        ks0108WriteChar(*string++);
    }

    return;
}
