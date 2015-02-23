#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

#define MUX_LEFT			0
#define MUX_RIGHT			1

#define DC_CORR				128

void adcInit();
uint8_t *getSpData(void);

#endif /* ADC_H */
