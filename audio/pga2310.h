#ifndef PGA2310_H
#define PGA2310_H

#include <inttypes.h>

// Number of inputs
#define PGA2310_IN_CNT              1

#define PGA2310_MUTE                0

void pga2310Init(void);
void pga2310SetSpeakers(void);
void pga2310SetMute(void);

#endif // PGA2310_H
