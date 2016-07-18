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

typedef enum {
	AUDIOPROC_TDA7439 = 0,
	AUDIOPROC_TDA7312,
	AUDIOPROC_TDA7313,
	AUDIOPROC_TDA7314,
	AUDIOPROC_TDA7315,
	AUDIOPROC_TDA7318,
	AUDIOPROC_PT2314,
	AUDIOPROC_TDA7448,
	AUDIOPROC_PT232X,
	AUDIOPROC_TEA6330,
	AUDIOPROC_PGA2310,
	AUDIOPROC_END
} audioProc;

void sndInit(uint8_t extFunc);

uint8_t sndInputCnt(void);

void sndSetInput(uint8_t input);
uint8_t sndGetInput(void);

void sndSetMute(uint8_t value);
uint8_t sndGetMute(void);

void sndSetLoudness(uint8_t value);
uint8_t sndGetLoudness(void);
void sndSetSurround(uint8_t value);
uint8_t sndGetSurround(void);
void sndSetEffect3d(uint8_t value);
uint8_t sndGetEffect3d(void);
void sndSetToneDefeat(uint8_t value);
uint8_t sndGetToneDefeat(void);

void sndNextParam(uint8_t *mode);
void sndChangeParam(uint8_t mode, int8_t diff);

void sndPowerOn(void);
void sndPowerOff(void);

#endif /* AUDIOPROC */
