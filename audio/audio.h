#ifndef AUDIO_H
#define AUDIO_H

#include <inttypes.h>

#define MUTE_ON					1
#define MUTE_OFF				0

#define LOUDNESS_ON				1
#define LOUDNESS_OFF			0

typedef struct {
	const int8_t min;
	const int8_t max;
	const int8_t step;
} sndGrid;

typedef struct {
	int8_t value;
	const sndGrid *grid;
	uint8_t *label;
	uint8_t icon;
	void (*set)(int8_t value);
} sndParam;

typedef enum {
	MODE_SND_VOLUME = 0,
	MODE_SND_BASS,
	MODE_SND_MIDDLE,
	MODE_SND_TREBLE,
	MODE_SND_PREAMP,
	MODE_SND_FRONTREAR,
	MODE_SND_BALANCE,
	MODE_SND_CENTER,
	MODE_SND_SUBWOOFER,
	MODE_SND_GAIN0,
	MODE_SND_GAIN1,
	MODE_SND_GAIN2,
	MODE_SND_GAIN3,

	MODE_SND_END
} sndMode;

#endif /* AUDIO_H */
