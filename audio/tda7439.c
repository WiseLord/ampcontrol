#include "tda7439.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

static void tda7439SetBMT(uint8_t param)
{
	int8_t val = sndPar[MODE_SND_BASS + param - TDA7439_BASS].value;

	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(param);
	I2CWriteByte(val > 0 ? 15 - val : 7 + val);
	I2CStop();

	return;
}

void tda7439SetSpeakers(void)
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

	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
	I2CWriteByte(-spRight);
	I2CWriteByte(-spLeft);
	I2CStop();

	return;
}

void tda7439SetBass(void)
{
	tda7439SetBMT(TDA7439_BASS);

	return;
}

void tda7439SetMiddle(void)
{
	tda7439SetBMT(TDA7439_MIDDLE);

	return;
}

void tda7439SetTreble(void)
{
	tda7439SetBMT(TDA7439_TREBLE);

	return;
}

void tda7439SetPreamp(void)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_PREAMP);
	I2CWriteByte(-sndPar[MODE_SND_PREAMP].value);
	I2CStop();

	return;
}

void tda7439SetGain(void)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_INPUT_GAIN);
	I2CWriteByte(sndPar[MODE_SND_GAIN0 + aproc.input].value);
	I2CStop();

	return;
}

void tda7439SetInput(void)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_INPUT_SELECT);
	I2CWriteByte(TDA7439_IN_CNT - 1 - aproc.input);
	I2CStop();

	tda7439SetGain();

	return;
}

void tda7439SetMute(void)
{
	if (aproc.mute) {
		I2CStart(TDA7439_I2C_ADDR);
		I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
		I2CWriteByte(TDA7439_SPEAKER_MUTE);
		I2CWriteByte(TDA7439_SPEAKER_MUTE);
		I2CStop();
	} else {
		tda7439SetSpeakers();
	}

	return;
}
