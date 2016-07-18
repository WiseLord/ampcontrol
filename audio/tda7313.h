#ifndef TDA7313_H
#define TDA7313_H

#include <inttypes.h>
#include "audio.h"

#define TDA7313_ADDR			0b10001000

#define CHAN_CNT				3

/* TDA7313 data bytes */
#define TDA7313_VOLUME			0x00
#define TDA7313_SP_FRONT_LEFT	0x80
#define TDA7313_SP_FRONT_RIGHT	0xA0
#define TDA7313_SP_REAR_LEFT	0xC0
#define TDA7313_SP_REAR_RIGHT	0xE0
#define TDA7313_SW				0x40
#define TDA7313_BASS			0x60
#define TDA7313_TREBLE			0x70

#define MUTE_ON					1
#define MUTE_OFF				0
#define MUTE_VAL				0x1F

uint8_t getChan(void);
uint8_t getMute(void);
uint8_t getLoudness(void);

void sndNextParam(uint8_t *mode);
void sndChangeParam(uint8_t mode, int8_t diff);

void setChan(uint8_t ch);
void nextChan(void);

void muteVolume(void);
void unmuteVolume(void);

void switchMute(void);
void switchLoudness(void);

void loadAudioParams(uint8_t **txtLabels);
void setAudioParams(void);
void saveAudioParams(void);

#endif /* TDA7313_H */
