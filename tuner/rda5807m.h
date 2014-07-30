#ifndef RDA5807M_H
#define RDA5807M_H

#include <inttypes.h>

#define RDA5807M_ADDR			0b00100000

void rda5807mInit(void);
void rda5807mSetFreq(uint16_t freq, uint8_t mono);
void rda5807mReadStatus(uint8_t *buf);

#endif /* RDA5807M_H */
