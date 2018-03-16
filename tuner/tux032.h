#ifndef TUX032_H
#define TUX032_H

#include <inttypes.h>

#define TUX032_I2C_ADDR         0b11000010

#define TUX032_STEREO           (1<<5)

#define TUX032_BUF_STEREO(buf)  (buf[3] & TUX032_STEREO)

#define TUX032_RDBUF_SIZE       4

void tux032Init();

void tux032SetFreq();

void tux032ReadStatus();

void tux032SetMute();

void tux032PowerOn();
void tux032PowerOff();

#endif // TUX032_H
