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

    return;
}

static void st7920SetPort(uint8_t data)
{
    if (data & (1 << 0)) PORT(ST7920_D0) |= ST7920_D0_LINE;
    else PORT(ST7920_D0) &= ~ST7920_D0_LINE;
    if (data & (1 << 1)) PORT(ST7920_D1) |= ST7920_D1_LINE;
    else PORT(ST7920_D1) &= ~ST7920_D1_LINE;
    if (data & (1 << 2)) PORT(ST7920_D2) |= ST7920_D2_LINE;
    else PORT(ST7920_D2) &= ~ST7920_D2_LINE;
    if (data & (1 << 3)) PORT(ST7920_D3) |= ST7920_D3_LINE;
    else PORT(ST7920_D3) &= ~ST7920_D3_LINE;
    if (data & (1 << 4)) PORT(ST7920_D4) |= ST7920_D4_LINE;
    else PORT(ST7920_D4) &= ~ST7920_D4_LINE;
    if (data & (1 << 5)) PORT(ST7920_D5) |= ST7920_D5_LINE;
    else PORT(ST7920_D5) &= ~ST7920_D5_LINE;
    if (data & (1 << 6)) PORT(ST7920_D6) |= ST7920_D6_LINE;
    else PORT(ST7920_D6) &= ~ST7920_D6_LINE;
    if (data & (1 << 7)) PORT(ST7920_D7) |= ST7920_D7_LINE;
    else PORT(ST7920_D7) &= ~ST7920_D7_LINE;

    return;
}

static void st7920SetDdrIn(void)
{
    DDR(ST7920_D0) &= ~ST7920_D0_LINE;
    DDR(ST7920_D1) &= ~ST7920_D1_LINE;
    DDR(ST7920_D2) &= ~ST7920_D2_LINE;
    DDR(ST7920_D3) &= ~ST7920_D3_LINE;
    DDR(ST7920_D4) &= ~ST7920_D4_LINE;
    DDR(ST7920_D5) &= ~ST7920_D5_LINE;
    DDR(ST7920_D6) &= ~ST7920_D6_LINE;
    DDR(ST7920_D7) &= ~ST7920_D7_LINE;

    return;
}

static void st7920SetDdrOut(void)
{
    DDR(ST7920_D0) |= ST7920_D0_LINE;
    DDR(ST7920_D1) |= ST7920_D1_LINE;
    DDR(ST7920_D2) |= ST7920_D2_LINE;
    DDR(ST7920_D3) |= ST7920_D3_LINE;
    DDR(ST7920_D4) |= ST7920_D4_LINE;
    DDR(ST7920_D5) |= ST7920_D5_LINE;
    DDR(ST7920_D6) |= ST7920_D6_LINE;
    DDR(ST7920_D7) |= ST7920_D7_LINE;

    return;
}

static uint8_t st7920ReadPin(void)
{
    uint8_t ret = 0;

    if (PIN(ST7920_D0) & ST7920_D0_LINE) ret |= (1 << 0);
    if (PIN(ST7920_D1) & ST7920_D1_LINE) ret |= (1 << 1);
    if (PIN(ST7920_D2) & ST7920_D2_LINE) ret |= (1 << 2);
    if (PIN(ST7920_D3) & ST7920_D3_LINE) ret |= (1 << 3);
    if (PIN(ST7920_D4) & ST7920_D4_LINE) ret |= (1 << 4);
    if (PIN(ST7920_D5) & ST7920_D5_LINE) ret |= (1 << 5);
    if (PIN(ST7920_D6) & ST7920_D6_LINE) ret |= (1 << 6);
    if (PIN(ST7920_D7) & ST7920_D7_LINE) ret |= (1 << 7);

    return ret;
}

static void st7920WriteCmd(uint8_t cmd)
{
    _delay_us(50);

    PORT(ST7920_RS) &= ~ST7920_RS_LINE;
    st7920SetPort(cmd);

    PORT(ST7920_E) |= ST7920_E_LINE;
    asm("nop");
    PORT(ST7920_E) &= ~ST7920_E_LINE;

    return;
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
        PORT(ST7920_RS) &= ~ST7920_RS_LINE;                 // Go to command mode
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
        PORT(ST7920_E) |= ST7920_E_LINE;                    // Strob
        asm("nop");
        PORT(ST7920_E) &= ~ST7920_E_LINE;

        // Prepare to read pins
        if (j == ST7920_PHASE_SET_ADDR) {
            st7920SetPort(0xFF);                            // Pull-up data lines
            st7920SetDdrIn();                               // Set data lines as inputs
        }
    }

    if (++j > ST7920_PHASE_READ_PORT) {
        j = 0;
        PORT(ST7920_RS) |= ST7920_RS_LINE;                  // Go to data mode
    }

    if (++br >= ST7920_MAX_BRIGHTNESS)                      // Loop brightness
        br = ST7920_MIN_BRIGHTNESS;

    if (br == _br) {
        PORT(ST7920_BCKL) &= ~ST7920_BCKL_LINE;             // Turn backlight off
    } else if (br == 0)
        PORT(ST7920_BCKL) |= ST7920_BCKL_LINE;              // Turn backlight on

    return;
}

void st7920Init(void)
{
    // Set control and data lines as outputs
    DDR(ST7920_RS) |= ST7920_RS_LINE;
    DDR(ST7920_E) |= ST7920_E_LINE;
    DDR(ST7920_PSB) |= ST7920_PSB_LINE;
    st7920SetDdrOut();

    // Set RW line to zero
#ifdef _atmega32
    DDR(ST7920_RW) |= ST7920_RW_LINE;
    PORT(ST7920_RW) &= ~ST7920_RW_LINE;
#endif

    PORT(ST7920_RS) &= ~ST7920_RS_LINE;
    PORT(ST7920_E) &= ~ST7920_E_LINE;
    // Switch display to parallel mode
    PORT(ST7920_PSB) |= ST7920_PSB_LINE;

    // Hardware reset
#ifdef _atmega32
    DDR(ST7920_RST) |= ST7920_RST_LINE;
    PORT(ST7920_RST) &= ~ST7920_RST_LINE;
    _delay_us(1);
    PORT(ST7920_RST) |= ST7920_RST_LINE;
#endif

    // Init display in graphics mode
    _delay_ms(40);
    st7920WriteCmd(ST7920_FUNCTION | ST7920_8BIT);
    st7920WriteCmd(ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR);
    st7920WriteCmd(ST7920_FUNCTION | ST7920_8BIT | ST7920_EXT_INSTR | ST7920_GRAPHIC);

    DDR(ST7920_BCKL)  |= ST7920_BCKL_LINE;

    return;
}

void st7920Clear()
{
    uint8_t i, j;

    for (i = 0; i < ST7920_SIZE_X / 4; i++) {
        for (j = 0; j < ST7920_SIZE_Y / 2; j++) {
            fb[i][j] = 0x00;
        }
    }

    return;
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

    return;
}

uint8_t st7920GetPins(void)
{
    return ~pins;
}
