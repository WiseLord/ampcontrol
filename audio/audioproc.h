#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>
#include "tda731x.h"
#include "tda7439.h"
#include "../pins.h"

typedef enum {
//	AUDIOPROC_TDA7312,
	AUDIOPROC_TDA7313,
//	AUDIOPROC_TDA7314,
	AUDIOPROC_TDA7318,
	AUDIOPROC_TDA7439
} audioProc;

void sndInit(uint8_t **txtLabels);

sndParam *sndParAddr(uint8_t index);

uint8_t sndInputCnt(void);

void sndSetInput(uint8_t input);
uint8_t sndGetInput(void);







uint8_t getMute(void);

void changeParam(uint8_t dispMode, int8_t diff);


void muteVolume(void);
void unmuteVolume(void);


void setAudioParams(void);
void saveAudioParams(void);

uint8_t getLoudness();

#endif /* AUDIOPROC */
