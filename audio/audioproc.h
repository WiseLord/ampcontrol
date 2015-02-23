#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>
#include "audio.h"

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

void loadAudioParams(uint8_t **txtLabels);
void setAudioParams(void);
void saveAudioParams(void);

uint8_t getLoudness();

#endif /* AUDIOPROC */
