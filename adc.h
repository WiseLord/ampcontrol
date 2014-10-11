#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

#define DC_CORR 128 /* Raw value from ADC when no signal */

#define DISP_MIN_BR			0
#define DISP_MAX_BR			48

/* Backlight port */
#define BCKL_DDR			DDRB
#define BCKL_PORT			PORTB
#define BCKL				(1<<PB3)

void adcInit();
uint8_t *getSpData();

void setDispBr(uint8_t br);

#endif /* ADC_H */
