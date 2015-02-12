#ifndef TDA7318_H
#define TDA7318_H

#include <inttypes.h>

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

typedef struct {
	int8_t value;
	int8_t min;
	int8_t max;
	uint8_t step;
	uint8_t *label;
	const uint8_t *icon;
	void (*set)(int8_t value);
} sndParam;

enum {
	SND_VOLUME,
	SND_BASS,
	SND_TREBLE,
	SND_FRONTREAR,
	SND_BALANCE,
	SND_GAIN0,
	SND_GAIN1,
	SND_GAIN2,
	SND_GAIN3
};

#define SND_PARAM_COUNT			9

sndParam *sndParAddr(uint8_t index);

uint8_t getChan(void);
uint8_t getMute(void);

void changeParam(sndParam *sndPar, int8_t diff);

void setChan(uint8_t ch);
void nextChan(void);

void muteVolume(void);
void unmuteVolume(void);

void switchMute(void);

void loadAudioParams(uint8_t **txtLabels);
void setAudioParams(void);
void saveAudioParams(void);

#endif /* TDA7318_H */
