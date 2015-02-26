#ifndef TUX032_H
#define TUX032_H

#include <inttypes.h>

#define TUX032_I2C_ADDR			0b11000010

#define TUX032_STEREO			(1<<5)

#define TUX032_BUF_STEREO(buf)	(buf[3] & TUX032_STEREO)

void tux032Init(void);

void tux032SetFreq(uint16_t freq);

void tux032GoStby();
void tux032ExitStby();

uint8_t *tux032ReadStatus(void);

#endif /* TUX032_H */
