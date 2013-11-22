#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

#define CORR_L 507 /* Raw value from ADC when no signal in left channel */
#define CORR_R 510 /* Raw value from ADC when no signal in right channel */

void adcInit();
uint8_t * getData();

#endif /* ADC_H */
