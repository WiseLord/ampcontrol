#include "st7920.h"

#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "../pins.h"

static volatile uint8_t pins;

static uint8_t fb[ST7920_SIZE_X / 4][ST7920_SIZE_Y / 2];
static uint8_t _br;

void st7920SetBrightness(uint8_t br)
{
    _br = br;
}

static void st7920SetPort(uint8_t data)
{
    if (data & (1 << 0)) SET(ST7920_D0);
    else CLR(ST7920_D0);
    if (data & (1 << 1)) SET(ST7920_D1);
    else CLR(ST7920_D1);
    if (data & (1 << 2)) SET(ST7920_D2);
    else CLR(ST7920_D2);
    if (data & (1 << 3)) SET(ST7920_D3);
    else CLR(ST7920_D3);
    if (data & (1 << 4)) SET(ST7920_D4);
    else CLR(ST7920_D4);
    if (data & (1 << 5)) SET(ST7920_D5);
    else CLR(ST7920_D5);
    if (data & (1 << 6)) SET(ST7920_D6);
    else CLR(ST7920_D6);
    if (data & (1 << 7)) SET(ST7920_D7);
    else CLR(ST7920_D7);
}

static void st7920SetDdrIn()
{
    IN(ST7920_D0);
    IN(ST7920_D1);
    IN(ST7920_D2);
    IN(ST7920_D3);
    IN(ST7920_D4);
    IN(ST7920_D5);
    IN(ST7920_D6);
    IN(ST7920_D7);
}

static void st7920SetDdrOut()
{
    OUT(ST7920_D0);
    OUT(ST7920_D1);
    OUT(ST7920_D2);
    OUT(ST7920_D3);
    OUT(ST7920_D4);
    OUT(ST7920_D5);
    OUT(ST7920_D6);
    OUT(ST7920_D7);
}

static uint8_t st7920ReadPin()
{
    uint8_t ret = 0;

    if (READ(ST7920_D0)) ret |= (1 << 0);
    if (READ(ST7920_D1)) ret |= (1 << 1);
    if (READ(ST7920_D2)) ret |= (1 << 2);
    if (READ(ST7920_D3)) ret |= (1 << 3);
    if (READ(ST7920_D4)) ret |= (1 << 4);
    if (READ(ST7920_D5)) ret |= (1 << 5);
    if (READ(ST7920_D6)) ret |= (1 << 6);
    if (READ(ST7920_D7)) ret |= (1 << 7);

    return ret;
}

static void st7920WriteCmd(uint8_t cmd)
{
    _delay_us(50);

    CLR(ST7920_RS);
    st7920SetPort(cmd);

    SET(ST7920_E);
    asm("nop");
    CLR(ST7920_E);
}

ISR (TIMER0_OVF_vect)
{
    // 2MHz / (256 - 156) = 20000Hz => 20000 / 32 / 34 = 18.4 FPS
    TCNT0 = 156;

    static uint8_t run = 1;
    if (run)
        ADCSRA |= 1 << ADSC;                                // Start ADC every second interrupt
    run = !run;

    static uint8_t i = 0;
    static uint8_t j = 32;

    static uint8_t br;

    if (j == ST7920_PHASE_SET_PAGE) {                       // Phase 1 (Y)
        CLR(ST7920_RS);                                     // Go to command mode
        if (++i >= ST7920_PHASE_SET_PAGE)
            i = 0;
        st7920SetPort(ST7920_SET_GRAPHIC_RAM | i);          // Set Y
    } else if (j == ST7920_PHASE_SET_ADDR) {                // Phase 2 (X)
        st7920SetPort(ST7920_SET_GRAPHIC_RAM);              // Set X
    } else if (j == ST7920_PHASE_READ_PORT) {
        pins = st7920ReadPin();                             // Read pins
        st7920SetDdrOut();                                  // Set data lines as outputs
    } else {                                                // Phase 3 (32 bytes of data)
        st7920SetPort(fb[j][i]);
    }

    if (j != ST7920_PHASE_READ_PORT) {
        SET(ST7920_E);                                      // Strob
        asm("nop");
        CLR(ST7920_E);

        // Prepare to read pins
        if (j == ST7920_PHASE_SET_ADDR) {
            st7920SetPort(0xFF);                            // Pull-up data lines
            st7920SetDdrIn();                               // Set data lines as inputs
        }
    }

    if (++j > ST7920_PHASE_READ_PORT) {
        j = 0;
        SET(ST7920_RS);                                     // Go to data mode
    }

    if (++br >= ST7920_MAX_BRIGHTNESS)                      // Loop brightness
        br = ST7920_MIN_BRIGHTNESS;

    if (br == _br) {
        CLR(ST7920_BCKL);                                   // Turn backlight off
    } else if (br == 0)
        SET(ST7920_BCKL);                                   // Turn backlight on
}

void st7920Init()
{
    // Set control and data lines as outputs
    OUT(ST7920_RS);
    OUT(ST7920_E);
    OUT(ST7920_PSB);
    st7920SetDdrOut();

    // Set RW line to zero
#ifdef _atmega32
    OUT(ST7920_RW);
    CLR(ST7920_RW);
#endif

    CLR(ST7920_RS);
    CLR(ST7920_E);
    // Switch display to parallel mode
    SET(ST7920_PSB);

    // Hardware reset
#ifdef _atmega32
    OUT(ST7920_RST);
    CLR(ST7920_RST);
    _delay_us(1);
    SET(ST7920_RST);
#endif

    // Init display in graphics mode
    _delay_ms(40);
    st7920WriteCmd(ST7920_FUNCTION | ST7920_8BIT);
    st7920WriteCmd(ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR);
    st7920WriteCmd(ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR | ST7920_GRAPHIC);

    OUT(ST7920_BCKL);
}

void st7920Clear()
{
    uint8_t i, j;

    for (i = 0; i < ST7920_SIZE_X / 4; i++) {
        for (j = 0; j < ST7920_SIZE_Y / 2; j++) {
            fb[i][j] = 0x00;
        }
    }
}

void st7920DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t bit;

    if (x >= ST7920_SIZE_X)
        return;
    if (y >= ST7920_SIZE_Y)
        return;

    bit = 0x80 >> (x & 0x07);

    if (y >= 32)
        x += 128;

    if (color)
        fb[x >> 3][y & 0x1F] |= bit;
    else
        fb[x >> 3][y & 0x1F] &= ~bit;
}

uint8_t st7920GetPins()
{
    return ~pins;
}
