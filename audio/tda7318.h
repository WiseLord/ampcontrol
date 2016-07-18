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

void sndInit(void);

uint8_t sndInputCnt(void);

void sndSetInput(uint8_t ch);
uint8_t sndGetInput(void);

void switchMute(void);
void muteVolume(void);
void unmuteVolume(void);
void sndSetMute(uint8_t value);
uint8_t sndGetMute(void);

void sndSetLoudness(uint8_t value);
uint8_t sndGetLoudness(void);

void sndNextParam(uint8_t *mode);
void sndChangeParam(uint8_t mode, int8_t diff);

void sndPowerOn(void);
void sndPowerOff(void);

#endif /* TDA7318_H */
