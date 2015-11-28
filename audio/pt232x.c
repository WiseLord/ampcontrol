#include "pt232x.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

static sndParam *sndPar;
static uint8_t _sndFunc;

static void pt2322SetSndFunc(void)
{
	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_FUNCTION | _sndFunc);
	I2CStop();

	return;
}

void pt232xInit(sndParam *sp)
{
	sndPar = sp;

	return;
}

void pt232xReset()
{
	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_CREAR_REGS);
	I2CStop();

	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_INPUT_SW);
	I2CStop();

	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_UNMUTE_ALL);
	I2CStop();

	return;
}

void pt2322SetVolume(int8_t val)
{
	val = -val;

	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_VOL_HI | (val / 10));
	I2CWriteByte(PT2322_VOL_LO | (val % 10));
	I2CStop();

	return;
}

void pt2322SetBass(int8_t val)
{
	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_BASS | (val > 0 ? 15 - val : 7 + val));
	I2CStop();

	return;
}

void pt2322SetMiddle(int8_t val)
{
	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_MIDDLE | (val > 0 ? 15 - val : 7 + val));
	I2CStop();

	return;
}

void pt2322SetTreble(int8_t val)
{
	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_TREBLE | (val > 0 ? 15 - val : 7 + val));
	I2CStop();

	return;
}

void pt2322SetSpeakers(int8_t val)
{

}

void pt2322SetMux(int8_t val)
{
	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_MUX | val);
	I2CStop();

	return;
}

void pt2323SetInput(uint8_t in)
{
	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_INPUT_SWITCH | (PT2323_INPUT_ST1 - in));
	I2CStop();

	pt2322SetMux(sndPar[MODE_SND_GAIN0 + in].value);

	return;
}

void pt232xSetMute(uint8_t val)
{
	if (val)
		_sndFunc |= PT2322_MUTE_ON;
	else
		_sndFunc &= ~PT2322_MUTE_ON;

	pt2322SetSndFunc();

	return;
}

void pt2323SetSurround(uint8_t val)
{
	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_ENH_SURR | !val);
	I2CStop();

	return;
}

void pt2322SetEffect3d(uint8_t val)
{
	if (val)
		_sndFunc &= ~PT2322_3D_OFF;
	else
		_sndFunc |= PT2322_3D_OFF;

	pt2322SetSndFunc();

	return;
}

void pt2322SetToneDefeat(uint8_t val)
{
	if (val)
		_sndFunc &= ~PT2322_TONE_OFF;
	else
		_sndFunc |= PT2322_TONE_OFF;

	pt2322SetSndFunc();

	return;
}
