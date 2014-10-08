#ifndef TDA7313_H
#define TDA7313_H

#include <inttypes.h>

#define AUDIOPROC_ADDR			0b10001000

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

#define LOUDNESS_ON				0
#define LOUDNESS_OFF			1

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
	SND_GAIN2
};

#define SND_PARAM_COUNT			8

sndParam *sndParAddr(uint8_t index);
uint8_t getChan();
uint8_t getMute();
uint8_t getLoudness();

void changeParam(sndParam *sndPar, int8_t diff);

void setChan(uint8_t ch);
void nextChan(void);

void muteVolume(void);
void unmuteVolume(void);

void switchMute(void);
void switchLoudness(void);

void loadAudioParams(uint8_t **txtLabels);
void saveAudioParams(void);

#endif /* TDA7313_H */
