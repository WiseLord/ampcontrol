#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>
#include "audio.h"

#ifdef _TDA7439
#include "tda7439.h"
#endif
#ifdef _TDA731X
#include "tda731x.h"
#endif
#ifdef _TDA7448
#include "tda7448.h"
#endif
#ifdef _PT232X
#include "pt232x.h"
#endif
#ifdef _TEA6330
#include "tea6330.h"
#endif
#ifdef _PGA2310
#include "pga2310.h"
#endif

void sndInit(uint8_t extFunc);

void sndSetInput(uint8_t input);

void sndSetMute(uint8_t value);

void sndSetLoudness(uint8_t value);
void sndSetSurround(uint8_t value);
void sndSetEffect3d(uint8_t value);
void sndSetToneDefeat(uint8_t value);

void sndNextParam(uint8_t *mode);
void sndChangeParam(uint8_t mode, int8_t diff);

void sndPowerOn(void);
void sndPowerOff(void);

#endif /* AUDIOPROC */
