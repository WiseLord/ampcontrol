#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

#define DC_CORR 128 /* Raw value from ADC when no signal */

#define ADC_TIMER_DISABLED	0
#define ADC_TIMER_ENABLED	1

#define DISP_MIN_BR			0
#define DISP_MAX_BR			32

/* Backlight port */
#define BCKL_DDR			DDRC
#define BCKL_PORT			PORTC
#define BCKL				(1<<PC7)

void adcInit();
uint8_t *getSpData();

void gdSetBrightness(uint8_t br);

#endif /* ADC_H */
