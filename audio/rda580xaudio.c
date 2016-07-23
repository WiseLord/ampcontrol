#include "rda580xaudio.h"
#include "audio.h"

#include "../pins.h"
#include "../tuner/tuner.h"
#include "../tuner/rda580x.h"

void rda580xAudioSetVolume(void)
{
	tuner.volume = sndPar[MODE_SND_VOLUME].value;
	rda580xSetAudio();

	return;
}

void rda580xAudioSetMute()
{
	tuner.mute = aproc.mute;
	rda580xSetAudio();

	return;
}

void rda580xAudioBass()
{
	tuner.bass = aproc.loudness;
	rda580xSetAudio();
}
