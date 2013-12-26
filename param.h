#ifndef PARAM_H
#define PARAM_H

#include <inttypes.h>

/* Function selection */
#define FUNC_INPUT_SELECT	0x00
#define FUNC_INPUT_GAIN		0x01
#define FUNC_PREAMP			0x02
#define FUNC_BASS			0x03
#define FUNC_MIDDLE			0x04
#define FUNC_TREBLE			0x05
#define FUNC_VOLUME_RIGHT	0x06
#define FUNC_VOLUME_LEFT	0x07

typedef enum {
	DISPLAY_SPECTRUM,
	DISPLAY_VOLUME,
	DISPLAY_BASS,
	DISPLAY_MIDDLE,
	DISPLAY_TREBLE,
	DISPLAY_PREAMP,
	DISPLAY_GAIN,
	DISPLAY_BALANCE,
	DISPLAY_TIME,
	DISPLAY_EDIT_TIME
} displayMode;

#define AMP_MIN		-47
#define AMP_MAX		0
#define BMT_MIN		-14
#define BMT_MAX		14
#define BAL_MIN		-21
#define BAL_MAX		21
#define VOL_MIN		-79
#define VOL_MAX		0

#define GAIN_MIN	0
#define GAIN_MAX	30

#define FAN_MIN		0
#define FAN_MAX		20

int8_t preamp;
int8_t bass;
int8_t middle;
int8_t treble;
int8_t balance;
int8_t volume;

uint8_t channel;
int8_t gain[4];

int8_t spMode;

void muteVolume();
void unmuteVolume();

void loadParams(void);
void saveParams(void);

void editSpMode();

void incVolume(void);
void decVolume(void);
void incBMT(int8_t *par);
void decBMT(int8_t *par);
void incBalance(void);
void decBalance(void);
void incSpeaker(void);
void decSpeaker(void);

void setChannel(uint8_t ch);
void incChannel(void);
void incGain(uint8_t chan);
void decGain(uint8_t chan);

void showVolume(uint8_t *parLabel);
void showBMT(int8_t *par, uint8_t *parLabel);
void showBalance(uint8_t *parLabel);
void showSpeaker(uint8_t *parLabel);
void showGain(uint8_t chan, uint8_t *parLabel);

#endif /* PARAM_H */
