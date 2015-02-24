#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>
#include "tda731x.h"
#include "tda7439.h"

typedef enum {
//	AUDIOPROC_TDA7312,
	AUDIOPROC_TDA7313,
//	AUDIOPROC_TDA7314,
	AUDIOPROC_TDA7318,
	AUDIOPROC_TDA7439
} audioProc;

void audioprocInit(uint8_t **txtLabels);







#define CHAN_CNT				4

#define MUTE_ON					1
#define MUTE_OFF				0

sndParam *sndParAddr(uint8_t index);

uint8_t getChan(void);
uint8_t getMute(void);

void changeParam(uint8_t dispMode, int8_t diff);

void setChan(uint8_t ch);
void nextChan(void);

void muteVolume(void);
void unmuteVolume(void);

void switchMute(void);

void setAudioParams(void);
void saveAudioParams(void);

uint8_t getLoudness();

#endif /* AUDIOPROC */
