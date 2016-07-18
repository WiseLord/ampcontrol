#ifndef TDA7318_H
#define TDA7318_H

#include <inttypes.h>
#include "audio.h"

#define TDA7318_ADDR			0b10001000

#define CHAN_CNT				4

/* TDA7318 data bytes */
#define TDA7318_VOLUME			0x00
#define TDA7318_SP_FRONT_LEFT	0x80
#define TDA7318_SP_FRONT_RIGHT	0xA0
#define TDA7318_SP_REAR_LEFT	0xC0
#define TDA7318_SP_REAR_RIGHT	0xE0
#define TDA7318_SW				0x40
#define TDA7318_BASS			0x60
#define TDA7318_TREBLE			0x70

#define MUTE_ON					1
#define MUTE_OFF				0

sndParam *sndParAddr(uint8_t index);

uint8_t getChan(void);
uint8_t getMute(void);
uint8_t getLoudness(void);

void changeParam(sndParam *sndPar, int8_t diff);

void setChan(uint8_t ch);
void nextChan(void);

void muteVolume(void);
void unmuteVolume(void);

void switchMute(void);
void switchLoudness(void);

void loadAudioParams(uint8_t **txtLabels);
void setAudioParams(void);
void saveAudioParams(void);

#endif /* TDA7318_H */
