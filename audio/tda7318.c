#include "tda7318.h"

#include <avr/eeprom.h>

#include "../i2c.h"
#include "../eeprom.h"
#include "../display.h"

static uint8_t chan;
static uint8_t mute;

sndParam sndPar[MODE_SND_END] = {
	{0x00, 0xC1, 0x00, 0x0A},	/* Volume */
	{0x00, 0xF9, 0x07, 0x10},	/* Bass */
	{},
	{0x00, 0xF9, 0x07, 0x10},	/* Treble */
	{},
	{0x00, 0xF1, 0x0F, 0x0A},	/* Front/Rear */
	{0x00, 0xF1, 0x0F, 0x0A},	/* Balance */
	{},
	{},
	{0x00, 0x00, 0x03, 0x32},	/* Gain 0 */
	{0x00, 0x00, 0x03, 0x32},	/* Gain 1 */
	{0x00, 0x00, 0x03, 0x32},	/* Gain 2 */
	{0x00, 0x00, 0x03, 0x32},	/* Gain 3 */
	{},
};

static void setNothing(int8_t value)
{
	return;
}

static void setVolume(int8_t val)
{
	int8_t spFrontLeft = 0;
	int8_t spFrontRight = 0;

	int8_t spRearLeft = 0;
	int8_t spRearRight = 0;

	if (sndPar[MODE_SND_BALANCE].value > 0) {
		spFrontRight -= sndPar[MODE_SND_BALANCE].value;
		spRearRight -= sndPar[MODE_SND_BALANCE].value;
	} else {
		spFrontLeft += sndPar[MODE_SND_BALANCE].value;
		spRearLeft += sndPar[MODE_SND_BALANCE].value;
	}
	if (sndPar[MODE_SND_FRONTREAR].value > 0) {
		spRearLeft -= sndPar[MODE_SND_FRONTREAR].value;
		spRearRight -= sndPar[MODE_SND_FRONTREAR].value;
	} else {
		spFrontLeft += sndPar[MODE_SND_FRONTREAR].value;
		spFrontRight += sndPar[MODE_SND_FRONTREAR].value;
	}
	I2CStart(TDA7318_ADDR);
	I2CWriteByte(TDA7318_VOLUME | -val);
	I2CWriteByte(TDA7318_SP_FRONT_LEFT | -spFrontLeft);
	I2CWriteByte(TDA7318_SP_FRONT_RIGHT | -spFrontRight);
	I2CWriteByte(TDA7318_SP_REAR_LEFT | -spRearLeft);
	I2CWriteByte(TDA7318_SP_REAR_RIGHT | -spRearRight);
	I2CStop();

	return;
}

static int8_t calcBMT(int8_t val)
{
	if (val > 0)
		return 15 - val;

	return 7 + val;
}

static void setBass(int8_t val)
{
	I2CStart(TDA7318_ADDR);
	I2CWriteByte(TDA7318_BASS | calcBMT(val));
	I2CStop();

	return;
}

static void setTreble(int8_t val)
{
	I2CStart(TDA7318_ADDR);
	I2CWriteByte(TDA7318_TREBLE | calcBMT(val));
	I2CStop();

	return;
}

static void setBalance(int8_t val)
{
	setVolume(sndPar[MODE_SND_VOLUME].value);

	return;
}

static void setGain(int8_t gain)
{
	I2CStart(TDA7318_ADDR);
	I2CWriteByte(TDA7318_SW | (CHAN_CNT - gain) << 3 | chan);
	I2CStop();

	return;
}


uint8_t sndGetInput(void)
{
	return chan;
}

void sndSetMute(uint8_t value)
{
	if (mute)
		setVolume(sndPar[MODE_SND_VOLUME].min);
	else
		setVolume(sndPar[MODE_SND_VOLUME].value);
}

uint8_t sndGetMute(void)
{
	return mute;
}

uint8_t sndGetLoudness(void)
{
	return 0;
}

void sndNextParam(uint8_t *mode)
{
	do {					/* Skip unused params (with step = 0) */
		(*mode)++;
		if (*mode >= MODE_SND_GAIN0)
			*mode = MODE_SND_VOLUME;
	} while((sndPar[*mode].step == 0) &&
			(*mode < MODE_SND_GAIN0) && (*mode != MODE_SND_VOLUME));

	return;
}

void sndChangeParam(uint8_t mode, int8_t diff)
{
	sndParam *param = &sndPar[mode];

	param->value += diff;
	if (param->value > param->max)
		param->value = param->max;
	if (param->value < param->min)
		param->value = param->min;
	param->set(param->value);

	return;
}


void sndSetInput(uint8_t ch)
{
	chan = ch;
	setGain(sndPar[MODE_SND_GAIN0 + chan].value);

	return;
}

void sndSetLoudness(uint8_t value)
{
	return;
}


void sndInit(void)
{
	uint8_t i;
	uint8_t **txtLabels = getTxtLabels();

	for (i = 0; i < MODE_SND_END; i++) {
		sndPar[i].value = eeprom_read_byte((uint8_t*)EEPROM_VOLUME + i);
		sndPar[i].label = txtLabels[MODE_SND_VOLUME + i];
		sndPar[i].set = setNothing;
	}

	chan = eeprom_read_byte((uint8_t*)EEPROM_INPUT);

	sndPar[MODE_SND_VOLUME].set = setVolume;
	sndPar[MODE_SND_BASS].set = setBass;
	sndPar[MODE_SND_TREBLE].set = setTreble;
	sndPar[MODE_SND_FRONTREAR].set = setBalance;
	sndPar[MODE_SND_BALANCE].set = setBalance;

	for (i = 0; i < CHAN_CNT; i++)
		sndPar[MODE_SND_GAIN0 + i].set = setGain;

	return;
}

void sndPowerOn(void)
{
	sndSetMute(1);
	sndSetInput(chan);
	setBass(sndPar[MODE_SND_BASS].value);
	setBalance(0);
	setTreble(sndPar[MODE_SND_TREBLE].value);
	sndSetMute(0);

	return;
}

void sndPowerOff(void)
{
	uint8_t i;

	for (i = 0; i < MODE_SND_END; i++)
		eeprom_update_byte((uint8_t*)EEPROM_VOLUME + i, sndPar[i].value);

	eeprom_update_byte((uint8_t*)EEPROM_INPUT, chan);

	return;
}

uint8_t sndInputCnt()
{
	return CHAN_CNT;
}
