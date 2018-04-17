#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#include "fft.h"

#define swap(x)     (__builtin_avr_swap(x))     //  Swaps nibbles in byte

#define N_DB        32

#define MUX_LEFT    0
#define MUX_RIGHT   1

extern uint8_t buf[FFT_SIZE];               // Previous results: left and right

void adcInit();
void getSpData(uint8_t fallSpeed);
uint16_t getSignalLevel();

#endif // ADC_H
