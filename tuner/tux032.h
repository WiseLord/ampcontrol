#ifndef TUX032_H
#define TUX032_H

#include <inttypes.h>

#define TUX032_ADDR				0b11000010

#define TUX032_STEREO			(1<<5)

#define TUX032_BUF_STEREO(buf)	(buf[3] & TUX032_STEREO)

#define TUX032_FREQ_MIN			7600
#define TUX032_FREQ_MAX			10800

void tux032Init(void);

void tux032SetFreq(uint16_t freq);

void tux032GoStby();
void tux032ExitStby();

uint8_t *tux032ReadStatus(void);

#endif /* TUX032_H */
