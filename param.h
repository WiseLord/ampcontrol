#ifndef PARAM_H
#define PARAM_H

#include <inttypes.h>

#define TDA7439_ADDR		0b10001000
#define TDA7313_ADDR		0b10001000

/* Function selection */
#define FUNC_INPUT_SELECT	0x00
#define FUNC_INPUT_GAIN		0x01
#define FUNC_PREAMP			0x02
#define FUNC_BASS			0x03
#define FUNC_MIDDLE			0x04
#define FUNC_TREBLE			0x05
#define FUNC_VOLUME_RIGHT	0x06
#define FUNC_VOLUME_LEFT	0x07

typedef struct {
	int8_t value;
	int8_t min;
	int8_t max;
	uint8_t step;
	void *label;
	void (*set)(int8_t value);
} regParam;

regParam volume;
regParam bass;
regParam middle;
regParam treble;
regParam preamp;
regParam balance;
regParam gain[4];

uint8_t chan;

void loadParams(void);
void saveParams(void);

void incParam(regParam *param);
void decParam(regParam *param);

void nextChan(void);
void setChan(uint8_t ch);

void muteVolume(void);
void unmuteVolume(void);

void showParam(regParam *param);

#endif /* PARAM_H */
