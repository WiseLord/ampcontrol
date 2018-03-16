#include "adc.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "eeprom.h"

static int16_t fr[FFT_SIZE];                            // Real values
static int16_t fi[FFT_SIZE];                            // Imaginary values
uint8_t buf[FFT_SIZE];                                  // Previous results: left and right

static const uint8_t hannTable[] PROGMEM = {
    0,   1,   3,   6,  10,  16,  22,  30,
    38,  48,  58,  69,  81,  93, 105, 118,
    131, 143, 156, 168, 180, 191, 202, 212,
    221, 229, 236, 242, 247, 251, 254, 255,
};

static const int16_t dbTable[N_DB - 1] PROGMEM = {
    1,    1,    2,    2,    3,    4,    6,    8,
    10,   14,   18,   24,   33,   44,   59,   78,
    105,  140,  187,  250,  335,  448,  599,  801,
    1071, 1432, 1915, 2561, 3425, 4580, 6125
};

void adcInit()
{
    // Enable ADC with prescaler 16
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);
    ADMUX |= (1 << ADLAR);                              // Adjust result to left (8bit ADC)
#ifdef _atmega32
    TIMSK |= (1 << TOIE0);                              // Enable Timer0 overflow interrupt
    TCCR0 |= (0 << CS02) | (1 << CS01) | (0 << CS00);   // Set timer prescaller to 8 (2MHz)
#else
    TIMSK0 |= (1 << TOIE0);                             // Enable Timer0 overflow interrupt
    TCCR0B |= (0 << CS02) | (1 << CS01) | (0 << CS00);  // Set timer prescaller to 8 (2MHz)
#endif
}

static uint8_t revBits(uint8_t x)
{
    x = ((x & 0x15) << 1) | ((x & 0x2A) >> 1);          // 00abcdef => 00badcfe
    x = (x & 0x0C) | swap(x & 0x33);                    // 00badcfe => 00fedcba

    return x;
}

static void getValues(uint8_t mux)
{
    uint8_t i = 0;

    ADMUX &= ~((1 << MUX2) | (1 << MUX1) | (1 << MUX0));
    ADMUX |= mux;

    for (i = 0; i < FFT_SIZE; i++) {
        while (!(ADCSRA & (1 << ADSC)));                // Wait for start measure
        while (ADCSRA & (1 << ADSC));                   // Wait for finish measure

        fi[i] = ADCH;                                   // Store in FI for futher handling
    }
}

static void prepareData()
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
}

static void cplx2dB(int16_t *fr, int16_t *fi)
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
}

void getSpData(uint8_t fallSpeed)
{
    uint8_t i;
    uint8_t *p;
    uint8_t mux;

    for (mux = MUX_LEFT; mux <= MUX_RIGHT; mux++) {
        getValues(mux);
        prepareData();
        fftRad4(fr, fi);
        cplx2dB(fr, fi);

        p = &buf[(mux - MUX_LEFT) * FFT_SIZE / 2];

        for (i = 0; i < FFT_SIZE / 2; i++) {
            (*p > fallSpeed) ? (*p -= fallSpeed) : (*p = 1);
            if ((*p)-- <= fr[i])
                *p = fr[i];
            p++;
        }
    }
}

uint16_t getSignalLevel()
{
    uint16_t ret = 0;
    uint8_t i;

    for (i = 0; i < sizeof(buf); i++)
        ret += buf[i];

    return ret * 3 / 64;
}
