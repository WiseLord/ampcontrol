#ifndef RDA_580X_AUDIO_H
#define RDA_580X_AUDIO_H

#include <inttypes.h>

/* Number of inputs */
#define RDA580X_IN_CNT				1

void rda580xAudioSetVolume(void);
void rda580xAudioSetMute(void);
void rda580xAudioBass(void);

#endif /* RDA_580X_AUDIO_H */
