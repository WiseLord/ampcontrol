#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "adc.h"
#include "fft.h"
#include "pins.h"

static int16_t fr[FFT_SIZE];                            // Real values for left channel
static int16_t fi[FFT_SIZE];                            // Imaginary values
static uint8_t buf[FFT_SIZE / 4];                       // Buffer with previous fft results

static const uint8_t hannTable[] PROGMEM = {
    0,   1,   3,   6,  10,  16,  22,  30,
    38,  48,  58,  69,  81,  93, 105, 118,
    131, 143, 156, 168, 180, 191, 202, 212,
    221, 229, 236, 242, 247, 251, 254, 255,
};

static const int16_t dbTable[N_DB] PROGMEM = {
    1,    2,    3,    6,
    10,   18,   33,   59,
    105,  187,  335,  599,
    1071, 1915, 3425, 6125
};

static uint8_t _br;

void adcInit()
{
    TCCR0 = (0 << CS02) | (1 << CS01) | (0 << CS00);

    // Enable ADC with prescaler 16
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);

    // Set mux to ADC2 with adjusting result to left
    ADMUX = (1 << ADLAR) | (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0);

    return;
}

ISR (TIMER0_OVF_vect)
{
    // 1MHz / (255 - 155) = 10000Hz => 10kHz Fourier analysis
    TCNT0 = 155;

    ADCSRA |= 1 << ADSC;

    static uint8_t br;

    if (++br >= DISP_MAX_BR)                            // Loop brightness
        br = DISP_MIN_BR;

    if (br >= _br)
        CLR(BCKL);                                      // Turn backlight off
    else
        SET(BCKL);                                      // Turn backlight on

    return;
};

static uint8_t revBits(uint8_t x)
{
    x = ((x & 0x15) << 1) | ((x & 0x2A) >> 1);          // 00abcdef => 00badcfe
    x = (x & 0x0C) | swap(x & 0x33);                    // 00badcfe => 00fedcba

    return x;
}

static void getValues()
{
    uint8_t i;

    for (i = 0; i < FFT_SIZE; i++) {
        while (!(ADCSRA & (1 << ADSC)));                // Wait for start measure
        while (ADCSRA & (1 << ADSC));                   // Wait for finish measure
        fi[i] = ADCH;
    }

    return;
}

static void prepareData(void)
{
    uint8_t i, j;
    int16_t dcOft = 0;
    uint8_t hw;

    // Calculate average DC offset
    for (i = 0; i < FFT_SIZE; i++)
        dcOft += fi[i];
    dcOft /= FFT_SIZE;

    // Move FI => FR with reversing bit order in index
    for (i = 0; i < FFT_SIZE; i++) {
        j = revBits(i);
        hw = pgm_read_byte(&hannTable[i < 32 ? i : 63 - i]);
        fr[j] = ((fi[i] - dcOft) * hw) >> 6;
        fi[i] = 0;
    }

    return;
}

void cplx2dB()
{
    uint8_t i, j;
    int16_t calc;

    for (i = 0; i < FFT_SIZE / 2; i++) {
        calc = ((int32_t)fr[i] * fr[i] + (int32_t)fi[i] * fi[i]) >> 13;

        for (j = 0; j < N_DB - 1; j++)
            if (calc <= pgm_read_word(&dbTable[j]))
                break;
        fr[i] = j;
    }

    return;
}

uint8_t *getSpData()
{
    uint8_t i;

    getValues();
    prepareData();
    fftRad4(fr, fi);
    cplx2dB();

    for (i = 0; i < FFT_SIZE / 4; i++) {
        if (fr[i] < buf[i])
            buf[i]--;
        else
            buf[i] = fr[i];
    }

    return buf;
}

void setDispBr(uint8_t br)
{
    _br = br;

    return;
}
