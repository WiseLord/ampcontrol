#include "tda7439.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

static sndParam *sndPar;

void tda7439Init(sndParam *sp)
{
	sndPar = sp;

	return;
}

void tda7439SetVolume(int8_t val)
{
	int8_t spLeft = val;
	int8_t spRight = val;
	int8_t volMin = pgm_read_byte(&sndPar[MODE_SND_VOLUME].grid->min);

	if (sndPar[MODE_SND_BALANCE].value > 0) {
		spLeft -= sndPar[MODE_SND_BALANCE].value;
		if (spLeft < volMin)
			spLeft = volMin;
	} else {
		spRight += sndPar[MODE_SND_BALANCE].value;
		if (spRight < volMin)
			spRight = volMin	;
	}

	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
	I2CWriteByte(-spRight);
	I2CWriteByte(-spLeft);
	I2CStop();

	return;
}

void tda7439SetBass(int8_t val)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_BASS);
	I2CWriteByte(val > 0 ? 15 - val : 7 + val);
	I2CStop();

	return;
}

void tda7439SetMiddle(int8_t val)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_MIDDLE);
	I2CWriteByte(val > 0 ? 15 - val : 7 + val);
	I2CStop();

	return;
}

void tda7439SetTreble(int8_t val)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_TREBLE);
	I2CWriteByte(val > 0 ? 15 - val : 7 + val);
	I2CStop();

	return;
}

void tda7439SetPreamp(int8_t val)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_PREAMP);
	I2CWriteByte(-val);
	I2CStop();

	return;
}

void tda7439SetBalance(int8_t val)
{
	tda7439SetVolume(sndPar[MODE_SND_VOLUME].value);

	return;
}

void tda7439SetGain(int8_t val)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_INPUT_GAIN);
	I2CWriteByte(val);
	I2CStop();

	return;
}

void tda7439SetInput(uint8_t in)
{
	I2CStart(TDA7439_I2C_ADDR);
	I2CWriteByte(TDA7439_INPUT_SELECT);
	I2CWriteByte(TDA7439_IN_CNT - 1 - in);
	I2CStop();

	tda7439SetGain(sndPar[MODE_SND_GAIN0 + in].value);

	return;
}

void tda7439SetMute(uint8_t val)
{
	int8_t pr;

	if (val == MUTE_ON)
		pr = pgm_read_byte(&sndPar[MODE_SND_PREAMP].grid->min);
	else
		pr = sndPar[MODE_SND_PREAMP].value;

	tda7439SetPreamp(pr);


	return;
}
