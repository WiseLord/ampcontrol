#include "ks0108.h"

#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "../pins.h"

static volatile uint8_t pins;

#ifdef _KS0108B
#define KS0108_SET_CS1(); \
    PORT(KS0108_CS1) &= ~KS0108_CS1_LINE; \
    PORT(KS0108_CS2) |= KS0108_CS2_LINE;
#define KS0108_SET_CS2(); \
    PORT(KS0108_CS2) &= ~KS0108_CS2_LINE; \
    PORT(KS0108_CS1) |= KS0108_CS1_LINE;
#else
#define KS0108_SET_CS1(); \
    PORT(KS0108_CS1) |= KS0108_CS1_LINE; \
    PORT(KS0108_CS2) &= ~KS0108_CS2_LINE;
#define KS0108_SET_CS2(x); \
    PORT(KS0108_CS2) |= KS0108_CS2_LINE; \
    PORT(KS0108_CS1) &= ~KS0108_CS1_LINE;
#endif

static uint8_t fb[KS0108_COLS * KS0108_CHIPS][KS0108_ROWS];
static uint8_t _br;

void ks0108SetBrightness(uint8_t br)
{
    _br = br;

    return;
}

static void ks0108SetPort(uint8_t data)
{
    if (data & (1 << 0)) PORT(KS0108_D0) |= KS0108_D0_LINE;
    else PORT(KS0108_D0) &= ~KS0108_D0_LINE;
    if (data & (1 << 1)) PORT(KS0108_D1) |= KS0108_D1_LINE;
    else PORT(KS0108_D1) &= ~KS0108_D1_LINE;
    if (data & (1 << 2)) PORT(KS0108_D2) |= KS0108_D2_LINE;
    else PORT(KS0108_D2) &= ~KS0108_D2_LINE;
    if (data & (1 << 3)) PORT(KS0108_D3) |= KS0108_D3_LINE;
    else PORT(KS0108_D3) &= ~KS0108_D3_LINE;
    if (data & (1 << 4)) PORT(KS0108_D4) |= KS0108_D4_LINE;
    else PORT(KS0108_D4) &= ~KS0108_D4_LINE;
    if (data & (1 << 5)) PORT(KS0108_D5) |= KS0108_D5_LINE;
    else PORT(KS0108_D5) &= ~KS0108_D5_LINE;
    if (data & (1 << 6)) PORT(KS0108_D6) |= KS0108_D6_LINE;
    else PORT(KS0108_D6) &= ~KS0108_D6_LINE;
    if (data & (1 << 7)) PORT(KS0108_D7) |= KS0108_D7_LINE;
    else PORT(KS0108_D7) &= ~KS0108_D7_LINE;

    return;
}

static void ks0108SetDdrIn(void)
{
    DDR(KS0108_D0) &= ~KS0108_D0_LINE;
    DDR(KS0108_D1) &= ~KS0108_D1_LINE;
    DDR(KS0108_D2) &= ~KS0108_D2_LINE;
    DDR(KS0108_D3) &= ~KS0108_D3_LINE;
    DDR(KS0108_D4) &= ~KS0108_D4_LINE;
    DDR(KS0108_D5) &= ~KS0108_D5_LINE;
    DDR(KS0108_D6) &= ~KS0108_D6_LINE;
    DDR(KS0108_D7) &= ~KS0108_D7_LINE;

    return;
}

static void ks0108SetDdrOut(void)
{
    DDR(KS0108_D0) |= KS0108_D0_LINE;
    DDR(KS0108_D1) |= KS0108_D1_LINE;
    DDR(KS0108_D2) |= KS0108_D2_LINE;
    DDR(KS0108_D3) |= KS0108_D3_LINE;
    DDR(KS0108_D4) |= KS0108_D4_LINE;
    DDR(KS0108_D5) |= KS0108_D5_LINE;
    DDR(KS0108_D6) |= KS0108_D6_LINE;
    DDR(KS0108_D7) |= KS0108_D7_LINE;

    return;
}

static uint8_t ks0108ReadPin(void)
{
    uint8_t ret = 0;

    if (PIN(KS0108_D0) & KS0108_D0_LINE) ret |= (1 << 0);
    if (PIN(KS0108_D1) & KS0108_D1_LINE) ret |= (1 << 1);
    if (PIN(KS0108_D2) & KS0108_D2_LINE) ret |= (1 << 2);
    if (PIN(KS0108_D3) & KS0108_D3_LINE) ret |= (1 << 3);
    if (PIN(KS0108_D4) & KS0108_D4_LINE) ret |= (1 << 4);
    if (PIN(KS0108_D5) & KS0108_D5_LINE) ret |= (1 << 5);
    if (PIN(KS0108_D6) & KS0108_D6_LINE) ret |= (1 << 6);
    if (PIN(KS0108_D7) & KS0108_D7_LINE) ret |= (1 << 7);

    return ret;
}

static void ks0108WriteCmd(uint8_t cmd)
{
    _delay_us(50);

    PORT(KS0108_DI) &= ~KS0108_DI_LINE;
    ks0108SetPort(cmd);

    PORT(KS0108_E) |= KS0108_E_LINE;
    asm("nop");
    PORT(KS0108_E) &= ~KS0108_E_LINE;

    return;
}

ISR (TIMER0_OVF_vect)
{
    // 2MHz / (256 - 156) = 20000Hz => 20000Hz / 8 / 2 / 66 = 18.9 FPS
    TCNT0 = 156;

    static uint8_t run = 1;
    if (run)
        ADCSRA |= 1 << ADSC;                        // Start ADC every second interrupt
    run = !run;

    static uint8_t i;
    static uint8_t j;
    static uint8_t cs;

    static uint8_t br;

    if (j == KS0108_PHASE_SET_PAGE) {               // Phase 1 (Y)
        if (++i >= 8) {
            i = 0;
            if (++cs >= KS0108_CHIPS)
                cs = 0;
            switch (cs) {
            case 1:
                KS0108_SET_CS2();
                break;
            default:
                KS0108_SET_CS1();
                break;
            }
        }
        PORT(KS0108_DI) &= ~KS0108_DI_LINE;         // Go to command mode
        ks0108SetPort(KS0108_SET_PAGE + i);
    } else if (j == KS0108_PHASE_SET_ADDR) {        // Phase 2 (X)
        ks0108SetPort(KS0108_SET_ADDRESS);
    } else if (j == KS0108_PHASE_READ_PORT) {
        pins = ks0108ReadPin();                     // Read pins
        ks0108SetDdrOut();                          // Set data lines as outputs
    } else {                                        // Phase 3 (32 bytes of data)
        ks0108SetPort(fb[j + 64 * cs][i]);
    }

    if (j != KS0108_PHASE_READ_PORT) {
        PORT(KS0108_E) |= KS0108_E_LINE;            // Strob
        asm("nop");
        PORT(KS0108_E) &= ~KS0108_E_LINE;

        // Prepare to read pins
        if (j == KS0108_PHASE_SET_ADDR) {
            ks0108SetPort(0xFF);                    // Pull-up data lines
            ks0108SetDdrIn();                       // Set data lines as inputs
        }
    }

    if (++j > KS0108_PHASE_READ_PORT) {
        j = 0;
        PORT(KS0108_DI) |= KS0108_DI_LINE;          // Go to data mode
    }

    if (++br >= KS0108_MAX_BRIGHTNESS)              // Loop brightness
        br = KS0108_MIN_BRIGHTNESS;

    if (br == _br) {
        PORT(KS0108_BCKL) &= ~KS0108_BCKL_LINE;     // Turn backlight off
    } else if (br == 0)
        PORT(KS0108_BCKL) |= KS0108_BCKL_LINE;      // Turn backlight on

    return;
}

void ks0108Init(void)
{
    // Set control and data lines as outputs
    DDR(KS0108_DI) |= KS0108_DI_LINE;
    DDR(KS0108_E) |= KS0108_E_LINE;
    DDR(KS0108_CS1) |= KS0108_CS1_LINE;
    DDR(KS0108_CS2) |= KS0108_CS2_LINE;

    ks0108SetDdrOut();

    // Set RW line to zero
#ifdef _atmega32
    DDR(KS0108_RW) |= KS0108_RW_LINE;
    PORT(KS0108_RW) &= ~(KS0108_RW_LINE);
#endif

    PORT(KS0108_DI) &= ~(KS0108_DI_LINE);
    PORT(KS0108_E) &= ~(KS0108_E_LINE);

    // Hardware reset
#ifdef _atmega32
    DDR(KS0108_RES) |= KS0108_RES_LINE;
    PORT(KS0108_RES) &= ~(KS0108_RES_LINE);
    _delay_us(1);
    PORT(KS0108_RES) |= KS0108_RES_LINE;
    _delay_us(1);
#endif

    // Init first controller
    KS0108_SET_CS1();
    ks0108WriteCmd(KS0108_DISPLAY_START_LINE);
    ks0108WriteCmd(KS0108_DISPLAY_ON);

    // Init second controller
    KS0108_SET_CS2();
    ks0108WriteCmd(KS0108_DISPLAY_START_LINE);
    ks0108WriteCmd(KS0108_DISPLAY_ON);

    // Go to data mode
    PORT(KS0108_DI) |= KS0108_DI_LINE;

    // Enable backlight control
    DDR(KS0108_BCKL) |= KS0108_BCKL_LINE;

    return;
}

void ks0108Clear(void)
{
    uint8_t i, j;

    for (i = 0; i < KS0108_COLS * KS0108_CHIPS; i++) {
        for (j = 0; j < KS0108_ROWS; j++) {
            fb[i][j] = 0x00;
        }
    }

    return;
}

void ks0108DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t bit;

    if (x >= KS0108_COLS * KS0108_CHIPS)
        return;
    if (y >= KS0108_ROWS * 8)
        return;

    bit = 1 << (y & 0x07);

    if (color)
        fb[x][y >> 3] |= bit;
    else
        fb[x][y >> 3] &= ~bit;

    return;
}

uint8_t ks0108GetPins(void)
{
    return ~pins;
}
