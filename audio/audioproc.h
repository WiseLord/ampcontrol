#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>

#define CHAN_CNT				4

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

typedef enum {
	MODE_SND_VOLUME,
	MODE_SND_BASS,
	MODE_SND_MIDDLE,
	MODE_SND_TREBLE,
	MODE_SND_PREAMP,
	MODE_SND_FRONTREAR,
	MODE_SND_BALANCE,
	MODE_SND_GAIN0,
	MODE_SND_GAIN1,
	MODE_SND_GAIN2,
	MODE_SND_GAIN3,
	MODE_SND_END
} sndMode;

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
