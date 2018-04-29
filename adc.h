#ifndef ADC_H
#define ADC_H

#include <inttypes.h>

#define swap(x) (__builtin_avr_swap(x))     //  Swaps nibbles in byte

#define N_DB                16

#define DISP_MIN_BR         0
#define DISP_MAX_BR         12

void adcInit();
uint8_t *getSpData();

void setDispBr(uint8_t br);

#endif // ADC_H
