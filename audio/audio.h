#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <inttypes.h>

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
	void (*set)(void);
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
	MODE_SND_GAIN4,

	MODE_SND_END
} sndMode;

extern sndParam sndPar[MODE_SND_END];

typedef enum {
	AUDIOPROC_NO = 0,
	AUDIOPROC_TDA7439,
	AUDIOPROC_TDA7312,
	AUDIOPROC_TDA7313,
	AUDIOPROC_TDA7314,
	AUDIOPROC_TDA7315,
	AUDIOPROC_TDA7318,
	AUDIOPROC_PT2314,
	AUDIOPROC_TDA7448,
	AUDIOPROC_PT232X,
	AUDIOPROC_TEA6300,
	AUDIOPROC_TEA6330,
	AUDIOPROC_PGA2310,

	AUDIOPROC_RDA580X,

	AUDIOPROC_END
} aprocIC;

typedef struct {
	aprocIC ic;
	uint8_t inCnt;
	uint8_t input;
	uint8_t loudness;
	uint8_t surround;
	uint8_t effect3d;
	uint8_t toneDefeat;
	uint8_t mute;
} Audioproc_type;

extern Audioproc_type aproc;

void sndInit(void);

void sndSetInput(uint8_t input);

void sndSetMute(uint8_t value);

void sndSetLoudness(uint8_t value);
void sndSetSurround(uint8_t value);
void sndSetEffect3d(uint8_t value);
void sndSetToneDefeat(uint8_t value);

void sndNextParam(uint8_t *mode);
void sndChangeParam(uint8_t mode, int8_t diff);

void sndPowerOn(void);
void sndPowerOff(void);

#endif /* AUDIOPROC */
