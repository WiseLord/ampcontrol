#ifndef TEA63X0_H
#define TEA63X0_H

#include <inttypes.h>

// I2C address
#define TEA63X0_I2C_ADDR            0b10000000

// I2C function selection
#define TEA63X0_VOLUME_LEFT         0x00
#define TEA63X0_VOLUME_RIGHT        0x01
#define TEA63X0_BASS                0x02
#define TEA63X0_TREBLE              0x03
#define TEA63X0_FADER               0x04
#define TEA63X0_AUDIO_SW            0x05

// I2C function bits
#define TEA63X0_MFN                 0x20
#define TEA63X0_FCH                 0x10
#define TEA63X0_GMU                 0x80
#define TEA6330_EQN                 0x40
#define TEA6300_SCC                 0x04
#define TEA6300_SCB                 0x02
#define TEA6300_SCA                 0x01

// Number of inputs
#define TEA6300_IN_CNT              3
#define TEA6330_IN_CNT              1

void tea63x0SetVolume();
void tea63x0SetBT();
void tea63x0SetSpeakers();
void tea63x0SetInputMute();

#endif // TEA63X0_H
