#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>
#include "tda731x.h"
#include "tda7439.h"
#include "../pins.h"

typedef enum {
	AUDIOPROC_TDA7439 = 0,
	AUDIOPROC_TDA7312,
	AUDIOPROC_TDA7313,
	AUDIOPROC_TDA7314,
	AUDIOPROC_TDA7318,
	AUDIOPROC_END
} audioProc;

void sndInit(void);

sndParam *sndParAddr(uint8_t index);

uint8_t sndInputCnt(void);

void sndSetInput(uint8_t input);
uint8_t sndGetInput(void);

void sndSetMute(uint8_t value);
uint8_t sndGetMute(void);

void sndSetLoudness(uint8_t value);
uint8_t sndGetLoudness(void);

void sndNextParam(uint8_t *mode);
void sndChangeParam(uint8_t mode, int8_t diff);

void sndPowerOn(void);
void sndPowerOff(void);

#endif /* AUDIOPROC */
