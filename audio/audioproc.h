#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>
#include "audio.h"

void sndInit(void);

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
