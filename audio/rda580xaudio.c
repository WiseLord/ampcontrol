#include "pga2310.h"
#include "audio.h"

#include "../pins.h"
#include "../tuner/rda580x.h"

void rda580xAudioSetVolume(void)
{
	rda580xSetVolume(sndPar[MODE_SND_VOLUME].value);

	return;
}

void rda580xAudioSetMute()
{
	rda580xSetMute(aproc.mute);

	return;
}

void rda580xAudioBass()
{
	rda580xSetBass(aproc.loudness);
}
