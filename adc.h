#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

#define DC_CORR 128 /* Raw value from ADC when no signal */

#define ADC_TIMER_DISABLED	0
#define ADC_TIMER_ENABLED	1

void adcInit();
uint8_t *getSpData();

#endif /* ADC_H */
