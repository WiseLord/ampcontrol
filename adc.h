#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

#define DC_CORR 512 /* Raw value from ADC when no signal */

void adcInit();
uint8_t * getData();

#endif /* ADC_H */
