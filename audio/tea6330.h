#ifndef TEA6330_H
#define TEA6330_H

#include <inttypes.h>

// I2C address
#define TEA6330_I2C_ADDR			0b10000000

// I2C function selection
#define TEA6330_VOLUME_LEFT			0x00
#define TEA6330_VOLUME_RIGHT		0x01
#define TEA6330_BASS				0x02
#define TEA6330_TREBLE				0x03
#define TEA6330_FADER				0x04
#define TEA6330_AUDIO_SW			0x05

// I2C function bits
#define TEA6330_MFN					0x20
#define TEA6330_FCH					0x10
#define TEA6330_GMU					0x80
#define TEA6330_EQN					0x40

/* Number of inputs */
#define TEA6330_IN_CNT				1

void tea6330SetVolume(void);
void tea6330SetBT(void);
void tea6330SetFrontRear(void);
void tea6330SetMute(void);

#endif // TEA6330_H
