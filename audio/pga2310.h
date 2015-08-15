#ifndef PGA2310_H
#define PGA2310_H

#include <inttypes.h>
#include "audio.h"

/* Number of inputs */
#define PGA2310_IN_CNT				1

#define PGA2310_MUTE				0

void pga2310Init(sndParam *sp);
void pga2310SetSpeakers(int8_t val);
void pga2310SetMute(uint8_t val);

#endif /* PGA2310_H */
