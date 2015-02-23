#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>
#include "tda7439.h"
#include "tda7313.h"
#include "tda7318.h"

typedef enum {
	AUDIOPROC_TDA7439,
	AUDIOPROC_TDA7313,
	AUDIOPROC_TDA7318
} audioProc;



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

void audioprocInit(uint8_t **txtLabels);
void setAudioParams(void);
void saveAudioParams(void);

uint8_t getLoudness();

#endif /* AUDIOPROC */
