#ifndef TDA731X_H
#define TDA731X_H

#include <inttypes.h>
#include "audio.h"

/* I2C address */
#define TDA731X_I2C_ADDR			0b10001000

/* I2C function selection */
#define TDA731X_VOLUME				0x00
#define TDA731X_SP_FRONT_LEFT		0x80
#define TDA731X_SP_FRONT_RIGHT		0xA0
#define TDA731X_SP_REAR_LEFT		0xC0
#define TDA731X_SP_REAR_RIGHT		0xE0
#define TDA731X_MUTE				0x1F
#define TDA731X_SW					0x40
#define TDA731X_BASS				0x60
#define TDA731X_TREBLE				0x70

/* Number of inputs */
#define TDA7312_IN_CNT				4
#define TDA7313_IN_CNT				3
#define TDA7314_IN_CNT				1
#define TDA7315_IN_CNT				1
#define TDA7318_IN_CNT				4
#define PT2314_IN_CNT				4

void tda731xInit(sndParam *sp);
void tda731xSetVolume(int8_t val);
void tda731xSetBass(int8_t val);
void tda731xSetTreble(int8_t val);
void tda731xSetBalance(int8_t val);
void tda731xSetGain(int8_t val);
void tda731xSetInput(uint8_t in);
void tda731xSetLoudness(uint8_t val);
void tda731xSetMute(uint8_t val);

#endif /* TDA731X_H */
