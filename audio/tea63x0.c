#include "tea63x0.h"
#include "audio.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

void tea63x0SetVolume()
{
	int8_t spLeft = sndPar[MODE_SND_VOLUME].value;
	int8_t spRight = sndPar[MODE_SND_VOLUME].value;
	int8_t volMin = pgm_read_byte(&sndPar[MODE_SND_VOLUME].grid->min);

	if (sndPar[MODE_SND_BALANCE].value > 0) {
		spLeft -= sndPar[MODE_SND_BALANCE].value;
		if (spLeft < volMin)
			spLeft = volMin;
	} else {
		spRight += sndPar[MODE_SND_BALANCE].value;
		if (spRight < volMin)
			spRight = volMin;
	}

	I2CStart(TEA63X0_I2C_ADDR);
	I2CWriteByte(TEA63X0_VOLUME_LEFT);
	I2CWriteByte(spRight + 53);							// -66dB..20dB => -33..10 grid => 20..53 raw
	I2CWriteByte(spLeft + 53);
	I2CStop();

	return;
}

void tea63x0SetBT()
{
	I2CStart(TEA63X0_I2C_ADDR);
	I2CWriteByte(TEA63X0_BASS);
	I2CWriteByte(sndPar[MODE_SND_BASS].value + 7);		// -4..5 grid => 3..12 raw
	I2CWriteByte(sndPar[MODE_SND_TREBLE].value + 7);	// -4..4 grid => 3..11 raw
	I2CStop();

	return;
}

void tea63x0SetSpeakers()
{
	int8_t spFR = sndPar[MODE_SND_FRONTREAR].value;

	// Front channels
	I2CStart(TEA63X0_I2C_ADDR);
	I2CWriteByte(TEA63X0_FADER);
	I2CWriteByte(TEA63X0_MFN | TEA63X0_FCH | (spFR < 0 ? 15 + spFR : 15));
	I2CStop();

	// Rear channels
	I2CStart(TEA63X0_I2C_ADDR);
	I2CWriteByte(TEA63X0_FADER);
	I2CWriteByte(TEA63X0_MFN | (spFR < 0 ? 15 : 15 - spFR));
	I2CStop();

	return;
}

void tea63x0SetInputMute(void)
{
	I2CStart(TEA63X0_I2C_ADDR);
	I2CWriteByte(TEA63X0_AUDIO_SW);
	I2CWriteByte((aproc.mute ? TEA63X0_GMU : 0) | (1 << aproc.input));
	I2CStop();

	return;
}
