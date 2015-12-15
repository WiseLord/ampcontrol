#include "pt232x.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

static uint8_t _sndFunc;
static uint8_t _input;

static void pt2322SetSndFunc(void)
{
	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_FUNCTION | _sndFunc);
	I2CStop();

	return;
}

static void pt2322SetBMT(uint8_t param) {
	int8_t val = sndPar[MODE_SND_BASS + ((param - PT2322_BASS) >> 4)].value;
	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(param | (val > 0 ? 15 - val : 7 + val));
	I2CStop();

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

void pt2322SetVolume(void)
{
	int8_t val = -sndPar[MODE_SND_VOLUME].value;

	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(PT2322_VOL_HI | (val / 10));
	I2CWriteByte(PT2322_VOL_LO | (val % 10));
	I2CStop();

	return;
}

void pt2322SetBass(void)
{
	pt2322SetBMT(PT2322_I2C_ADDR);

	return;
}

void pt2322SetMiddle(void)
{
	pt2322SetBMT(PT2322_MIDDLE);

	return;
}

void pt2322SetTreble(void)
{
	pt2322SetBMT(PT2322_TREBLE);

	return;
}

void pt2322SetSpeakers(void)
{
	uint8_t i;
	int8_t sp[PT2322_CH_END] = {0, 0, 0, 0, 0, 0};

	if (sndPar[MODE_SND_BALANCE].value > 0) {
		sp[PT2322_CH_FL] += sndPar[MODE_SND_BALANCE].value;
		sp[PT2322_CH_RL] += sndPar[MODE_SND_BALANCE].value;
	} else {
		sp[PT2322_CH_FR] -= sndPar[MODE_SND_BALANCE].value;
		sp[PT2322_CH_RR] -= sndPar[MODE_SND_BALANCE].value;
	}

	if (sndPar[MODE_SND_FRONTREAR].value > 0) {
		sp[PT2322_CH_RL] += sndPar[MODE_SND_FRONTREAR].value;
		sp[PT2322_CH_RR] += sndPar[MODE_SND_FRONTREAR].value;
	} else {
		sp[PT2322_CH_FL] -= sndPar[MODE_SND_FRONTREAR].value;
		sp[PT2322_CH_FR] -= sndPar[MODE_SND_FRONTREAR].value;
	}

	sp[PT2322_CH_CT] = -sndPar[MODE_SND_CENTER].value;
	sp[PT2322_CH_SB] = -sndPar[MODE_SND_SUBWOOFER].value;

	I2CStart(PT2323_I2C_ADDR);
	for (i = 0; i < PT2322_CH_END; i++) {
		// PT2322_TRIM_XX
		I2CWriteByte(((i + 1) << 4) | sp[i]);
	}
	I2CStop();

	return;
}

void pt2322SetMux(void)
{
	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_MUX | sndPar[MODE_SND_GAIN0 + _input].value);
	I2CStop();

	return;
}

void pt2323SetInput(uint8_t in)
{
	_input = in;

	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_INPUT_SWITCH | (PT2323_INPUT_ST1 - in));
	I2CStop();

	pt2322SetMux();

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
