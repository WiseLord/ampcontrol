#ifndef TDA7313_H
#define TDA7313_H

#include <inttypes.h>
#include "../pins.h"

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
	SND_TREBLE,
	SND_BALANCE,
	SND_FRONTREAR,
	SND_GAIN0,
	SND_GAIN1,
	SND_GAIN2
};

#define SND_PARAM_COUNT			8

/* Input control */
#define IN0					ISP_MOSI
#define IN0_LINE			ISP_MOSI_LINE
#define IN1					ISP_MISO
#define IN1_LINE			ISP_MISO_LINE
#define IN2					ISP_SCK
#define IN2_LINE			ISP_SCK_LINE

sndParam *sndParAddr(uint8_t index);

uint8_t getChan(void);
uint8_t getMute(void);
uint8_t getLoudness(void);

void changeParam(uint8_t index, int8_t diff);

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
