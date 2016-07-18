#ifndef AUDIO_H
#define AUDIO_H

typedef struct {
	int8_t value;
	int8_t min;
	int8_t max;
	uint8_t step;
	uint8_t *label;
	void (*set)(int8_t value);
} sndParam;

enum {
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
	MODE_SND_GAIN4,

	MODE_SND_END
};

extern sndParam sndPar[MODE_SND_END];

/* Integral circuits definitions */
#if !defined(TDA7313) && !defined(TDA7318) && !defined(TDA7439)
#define TDA7313
#endif

#if defined(TDA7313)
#include "tda7313.h"
#elif defined(TDA7318)
#include "tda7318.h"
#elif defined(TDA7439)
#include "tda7439.h"
#endif

#endif /* AUDIO_H */
