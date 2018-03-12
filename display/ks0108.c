#include "ks0108.h"

#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "../pins.h"

static volatile uint8_t pins;

#ifdef _KS0108B
#define KS0108_SET_CS1(); \
    CLR(KS0108_CS1); \
    SET(KS0108_CS2);
#define KS0108_SET_CS2(); \
    CLR(KS0108_CS2); \
    SET(KS0108_CS1);
#else
#define KS0108_SET_CS1(); \
    SET(KS0108_CS1); \
    CLR(KS0108_CS2);
#define KS0108_SET_CS2(); \
    SET(KS0108_CS2); \
    CLR(KS0108_CS1);
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
    if (data & (1 << 0)) SET(KS0108_D0);
    else CLR(KS0108_D0);
    if (data & (1 << 1)) SET(KS0108_D1);
    else CLR(KS0108_D1);
    if (data & (1 << 2)) SET(KS0108_D2);
    else CLR(KS0108_D2);
    if (data & (1 << 3)) SET(KS0108_D3);
    else CLR(KS0108_D3);
    if (data & (1 << 4)) SET(KS0108_D4);
    else CLR(KS0108_D4);
    if (data & (1 << 5)) SET(KS0108_D5);
    else CLR(KS0108_D5);
    if (data & (1 << 6)) SET(KS0108_D6);
    else CLR(KS0108_D6);
    if (data & (1 << 7)) SET(KS0108_D7);
    else CLR(KS0108_D7);

    return;
}

static void ks0108SetDdrIn(void)
{
    IN(KS0108_D0);
    IN(KS0108_D1);
    IN(KS0108_D2);
    IN(KS0108_D3);
    IN(KS0108_D4);
    IN(KS0108_D5);
    IN(KS0108_D6);
    IN(KS0108_D7);

    return;
}

static void ks0108SetDdrOut(void)
{
    OUT(KS0108_D0);
    OUT(KS0108_D1);
    OUT(KS0108_D2);
    OUT(KS0108_D3);
    OUT(KS0108_D4);
    OUT(KS0108_D5);
    OUT(KS0108_D6);
    OUT(KS0108_D7);

    return;
}

static uint8_t ks0108ReadPin(void)
{
    uint8_t ret = 0;

    if (READ(KS0108_D0)) ret |= (1 << 0);
    if (READ(KS0108_D1)) ret |= (1 << 1);
    if (READ(KS0108_D2)) ret |= (1 << 2);
    if (READ(KS0108_D3)) ret |= (1 << 3);
    if (READ(KS0108_D4)) ret |= (1 << 4);
    if (READ(KS0108_D5)) ret |= (1 << 5);
    if (READ(KS0108_D6)) ret |= (1 << 6);
    if (READ(KS0108_D7)) ret |= (1 << 7);

    return ret;
}

static void ks0108WriteCmd(uint8_t cmd)
{
    _delay_us(50);

    CLR(KS0108_DI);
    ks0108SetPort(cmd);

    SET(KS0108_E);
    asm("nop");
    CLR(KS0108_E);

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
        CLR(KS0108_DI);                             // Go to command mode
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
        SET(KS0108_E);                              // Strob
        asm("nop");
        CLR(KS0108_E);

        // Prepare to read pins
        if (j == KS0108_PHASE_SET_ADDR) {
            ks0108SetPort(0xFF);                    // Pull-up data lines
            ks0108SetDdrIn();                       // Set data lines as inputs
        }
    }

    if (++j > KS0108_PHASE_READ_PORT) {
        j = 0;
        SET(KS0108_DI);                             // Go to data mode
    }

    if (++br >= KS0108_MAX_BRIGHTNESS)              // Loop brightness
        br = KS0108_MIN_BRIGHTNESS;

    if (br == _br) {
        CLR(KS0108_BCKL);                           // Turn backlight off
    } else if (br == 0)
        SET(KS0108_BCKL);                           // Turn backlight on

    return;
}

void ks0108Init(void)
{
    // Set control and data lines as outputs
    OUT(KS0108_DI);
    OUT(KS0108_E);
    OUT(KS0108_CS1);
    OUT(KS0108_CS2);

    ks0108SetDdrOut();

    // Set RW line to zero
#ifdef _atmega32
    OUT(KS0108_RW);
    CLR(KS0108_RW);
#endif

    CLR(KS0108_DI);
    CLR(KS0108_E);

    // Hardware reset
#ifdef _atmega32
    OUT(KS0108_RES);
    CLR(KS0108_RES);
    _delay_us(1);
    SET(KS0108_RES);
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
    SET(KS0108_DI);

    // Enable backlight control
    OUT(KS0108_BCKL);

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
