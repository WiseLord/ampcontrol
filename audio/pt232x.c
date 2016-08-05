#include "pt232x.h"
#include "audio.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

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

void pt2322SetBMT(void)
{
	int8_t val;
	uint8_t mode = MODE_SND_BASS;
	uint8_t param = PT2322_BASS;

	I2CStart(PT2322_I2C_ADDR);
	while (mode <= MODE_SND_TREBLE) {
		val = sndPar[mode++].value;
		I2CWriteByte(param | (val > 0 ? 15 - val : 7 + val));
		param += 0b00010000;
	}
	I2CStop();

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

	I2CStart(PT2322_I2C_ADDR);
	for (i = 0; i < PT2322_CH_END; i++) {
		// PT2322_TRIM_XX
		I2CWriteByte(((i + 1) << 4) | sp[i]);
	}
	I2CStop();

	return;
}

void pt2323SetInput(void)
{
	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_INPUT_SWITCH | (PT2323_INPUT_ST1 - aproc.input));
	I2CWriteByte(PT2323_MIX | sndPar[MODE_SND_GAIN0 + aproc.input].value);
	I2CStop();

	return;
}

void pt232xSetSndFunc(void)
{
	uint8_t sndFunc = PT2322_FUNCTION;

	if (aproc.mute)
		sndFunc |= PT2322_MUTE_ON;
	if (!(aproc.extra & APROC_EXTRA_EFFECT3D))
		sndFunc |= PT2322_3D_OFF;
	if (aproc.extra & APROC_EXTRA_TONEDEFEAT)
		sndFunc |= PT2322_TONE_OFF;

	I2CStart(PT2322_I2C_ADDR);
	I2CWriteByte(sndFunc);
	I2CStop();

	I2CStart(PT2323_I2C_ADDR);
	I2CWriteByte(PT2323_ENH_SURR | !(aproc.extra & APROC_EXTRA_SURROUND));
	I2CStop();

	return;
}
