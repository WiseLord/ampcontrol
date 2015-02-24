#ifndef TDA7439_H
#define TDA7439_H

#include <inttypes.h>
#include "audio.h"

void tda7439SetVolume(int8_t val);
void tda7439SetBass(int8_t val);
void tda7439SetMiddle(int8_t val);
void tda7439SetTreble(int8_t val);
void tda7439SetPreamp(int8_t val);
void tda7439SetBalance(int8_t val);
void tda7439SetGain(int8_t val);

#endif /* TDA7439_H */
