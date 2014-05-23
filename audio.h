#ifndef AUDIO_H
#define AUDIO_H

#include <inttypes.h>

#define AUDIOPROC_ADDR			0b10001000

/* Integral circuits definitions */
#if !defined(TDA7439) && !defined(TDA7313) && !defined(TDA7318)
#define TDA7439
#endif

/* Number of input channels */
#if defined(TDA7439) || defined(TDA7318)
#define CHAN_CNT				4
#elif defined(TDA7313)
#define CHAN_CNT				3
#endif

/* TDA7439 I2C function selection */
#define TDA7439_INPUT_SELECT	0x00
#define TDA7439_INPUT_GAIN		0x01
#define TDA7439_PREAMP			0x02
#define TDA7439_BASS			0x03
#define TDA7439_MIDDLE			0x04
#define TDA7439_TREBLE			0x05
#define TDA7439_VOLUME_RIGHT	0x06
#define TDA7439_VOLUME_LEFT		0x07
/* I2c autoincrement flag */
#define TDA7439_AUTO_INC		0x10

/* TDA7313 (7318) data bytes */
#define TDA7313_VOLUME	0x00
#define TDA7313_SP_FRONT_LEFT	0x80
#define TDA7313_SP_FRONT_RIGHT	0xA0
#define TDA7313_SP_REAR_LEFT	0xC0
#define TDA7313_SP_REAR_RIGHT	0xE0
#define TDA7313_SW	0x40
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
	void (*set)(int8_t value);
} sndParam;

enum {
	SND_VOLUME,
	SND_BASS,
#ifdef TDA7439
	SND_MIDDLE,
#endif
	SND_TREBLE,
#ifdef TDA7439
	SND_PREAMP,
#else
	SND_BALANCE_FR,
#endif
	SND_BALANCE,
	SND_GAIN0,
	SND_GAIN1,
	SND_GAIN2,
	SND_GAIN3
};

#define SND_PARAM_COUNT			10

sndParam sndPar[SND_PARAM_COUNT];

uint8_t chan;
uint8_t loud;
uint8_t mute;

void loadAudioParams(uint8_t **txtLabels);
void saveAudioParams(void);

void changeParam(sndParam *sndPar, int8_t diff);

void nextChan(void);
void setChan(uint8_t ch);

void muteVolume(void);
void unmuteVolume(void);

void switchMute(void);
void switchLoudness(void);

#endif /* AUDIO_H */
