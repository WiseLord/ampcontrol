#ifndef TDA7448_H
#define TDA7448_H

#include <inttypes.h>
#include "audio.h"

/* I2C address */
#define TDA7448_I2C_ADDR			0b10001000

/* Number of inputs */
#define TDA7448_IN_CNT				1

void tda7448Init(sndParam *sp);
void tda7448SetVolume(int8_t val);
void tda7448SetFrontRear(int8_t val);
void tda7448SetBalance(int8_t val);
void tda7448SetCenter(int8_t val);
void tda7448SetSubwoofer(int8_t val);

#endif /* TDA7448_H */
