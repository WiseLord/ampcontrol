#ifndef RDA5807X_H
#define RDA5807X_H

#include <inttypes.h>

#include "rda580x_regs.h"

#define RDA5807M_I2C_SEQ_ADDR       0x20
#define RDA5807M_I2C_RAND_ADDR      0x22

#define RDA5807_BUF_READY(buf)      (buf[3] & RDA580X_FM_READY)
#define RDA5807_BUF_STEREO(buf)     (buf[0] & RDA580X_ST)

#define RDA5807_RDBUF_SIZE          12
#define RDA5807_WRBUF_SIZE          14

#define RDA5807_VOL_MIN             0
#define RDA5807_VOL_MAX             16

void rda580xInit(void);

void rda580xSetFreq(void);

void rda580xReadStatus(void);

void rda580xSetVolume(int8_t value);
void rda580xSetMute(uint8_t value);
void rda580xSetBass(uint8_t value);

void rda580xSetMono(uint8_t value);
#ifdef _RDS
void rda580xSetRds(uint8_t value);
#endif
void rda580xSetPower(uint8_t value);

void rda580xSeek(int8_t direction);

#endif // RDA5807X_H
