#ifndef TDA7439_H
#define TDA7439_H

#include <inttypes.h>
#include "audio.h"

/* I2C address */
#define TDA7439_I2C_ADDR			0b10001000

/* I2C function selection */
#define TDA7439_INPUT_SELECT		0x00
#define TDA7439_INPUT_GAIN			0x01
#define TDA7439_PREAMP				0x02
#define TDA7439_BASS				0x03
#define TDA7439_MIDDLE				0x04
#define TDA7439_TREBLE				0x05
#define TDA7439_VOLUME_RIGHT		0x06
#define TDA7439_VOLUME_LEFT			0x07

/* I2C autoincrement flag */
#define TDA7439_AUTO_INC			0x10

/* Number of inputs */
#define TDA7439_IN_CNT				4

void tda7439Init(sndParam *sp);
void tda7439SetVolume(int8_t val);
void tda7439SetBass(int8_t val);
void tda7439SetMiddle(int8_t val);
void tda7439SetTreble(int8_t val);
void tda7439SetPreamp(int8_t val);
void tda7439SetBalance(int8_t val);
void tda7439SetGain(int8_t val);
void tda7439SetInput(uint8_t in);
void tda7439SetMute(uint8_t val);

#endif /* TDA7439_H */
