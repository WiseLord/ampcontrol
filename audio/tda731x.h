#ifndef TDA731X_H
#define TDA731X_H

#include <inttypes.h>
#include "audio.h"

void tda731xSetVolume(int8_t val);
void tda731xSetBass(int8_t val);
void tda731xSetTreble(int8_t val);
void tda731xSetFrontrear(int8_t val);
void tda731xSetBalance(int8_t val);
void tda731xSetGain(int8_t val);

#endif /* TDA731X_H */
