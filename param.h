#ifndef PARAM_H
#define PARAM_H

#include <inttypes.h>

typedef enum {
	DISPLAY_SPECTRUM,
	DISPLAY_VOLUME,
	DISPLAY_BASS,
	DISPLAY_MIDDLE,
	DISPLAY_TREBLE,
	DISPLAY_TIME
} displayMode;

#define VOL_MIN		-47
#define VOL_MAX		0

#define BMT_MIN		-14
#define BMT_MAX		14

int8_t volume;
int8_t bass;
int8_t middle;
int8_t treble;

void loadParams(void);
void saveParams(void);

void incVolume(void);
void decVolume(void);
void incBMT(int8_t *par);
void decBMT(int8_t *par);

void showVolume(uint8_t *parLabel);
void showBMT(int8_t par, uint8_t *parLabel);

#endif /* PARAM_H */
