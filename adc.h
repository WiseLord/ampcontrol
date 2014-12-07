#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

void adcInit();
uint8_t *getSpData();

#endif /* ADC_H */
