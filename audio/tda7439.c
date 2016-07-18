#include "tda7439.h"

#include <avr/eeprom.h>

#include "../i2c.h"
#include "../eeprom.h"
#include "../pins.h"

static uint8_t chan;
static uint8_t mute;

sndParam sndPar[MODE_SND_END] = {
	{0x00, 0xB1, 0x00, 0x08},	/* Volume */
	{0x00, 0xF9, 0x07, 0x10},	/* Bass */
	{0x00, 0xF9, 0x07, 0x10},	/* Middle */
	{0x00, 0xF9, 0x07, 0x10},	/* Treble */
	{0x00, 0xD1, 0x00, 0x08},	/* Preamp */
	{},
	{0x00, 0xEB, 0x15, 0x08},	/* Balance */
	{},
	{},
	{0x00, 0x00, 0x0F, 0x10},	/* Gain 0 */
	{0x00, 0x00, 0x0F, 0x10},	/* Gain 1 */
	{0x00, 0x00, 0x0F, 0x10},	/* Gain 2 */
	{0x00, 0x00, 0x0F, 0x10},	/* Gain 3 */
	{},
};

static void setNothing(int8_t value)
{
	return;
}

static void setVolume(int8_t val)
{
	int8_t spLeft = val;
	int8_t spRight = val;

	if (sndPar[MODE_SND_BALANCE].value > 0) {
		spLeft -= sndPar[MODE_SND_BALANCE].value;
		if (spLeft < sndPar[MODE_SND_VOLUME].min)
			spLeft = sndPar[MODE_SND_VOLUME].min;
	} else {
		spRight += sndPar[MODE_SND_BALANCE].value;
		if (spRight < sndPar[MODE_SND_VOLUME].min)
			spRight = sndPar[MODE_SND_VOLUME].min;
	}
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
	I2CWriteByte(-spRight);
	I2CWriteByte(-spLeft);
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
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_BASS);
	I2CWriteByte(calcBMT(val));
	I2CStop();

	return;
}

static void setMiddle(int8_t val)
{
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_MIDDLE);
	I2CWriteByte(calcBMT(val));
	I2CStop();

	return;
}

static void setTreble(int8_t val)
{
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_TREBLE);
	I2CWriteByte(calcBMT(val));
	I2CStop();

	return;
}

static void setPreamp(int8_t val)
{
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_PREAMP);
	I2CWriteByte(-val);
	I2CStop();

	return;
}

static void setBalance(int8_t val)
{
	setVolume(sndPar[MODE_SND_VOLUME].value);

	return;
}

static void setGain(int8_t val)
{
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_INPUT_GAIN);
	I2CWriteByte(val);
	I2CStop();

	return;
}


uint8_t getChan(void)
{
	return chan;
}

uint8_t getMute(void)
{
	return mute;
}

uint8_t getLoudness(void)
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


void setChan(uint8_t ch)
{
	chan = ch;
	setGain(sndPar[MODE_SND_GAIN0 + ch].value);

	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_INPUT_SELECT);
	I2CWriteByte(CHAN_CNT - 1 - ch);
	I2CStop();

	return;
}

void nextChan(void)
{
	chan++;
	if (chan >= CHAN_CNT)
		chan = 0;
	setChan(chan);

	return;
}


void muteVolume(void)
{
	setVolume(sndPar[MODE_SND_VOLUME].min);
	mute = MUTE_ON;

	return;
}

void unmuteVolume(void)
{
	setVolume(sndPar[MODE_SND_VOLUME].value);
	mute = MUTE_OFF;

	return;
}


void switchMute(void)
{
	if (mute == MUTE_ON) {
		unmuteVolume();
	} else {
		muteVolume();
	}

	return;
}

void switchLoudness(void)
{
	return;
}

void loadAudioParams(uint8_t **txtLabels)
{
	uint8_t i;

	for (i = 0; i < MODE_SND_END; i++) {
		sndPar[i].value = eeprom_read_byte((uint8_t*)EEPROM_VOLUME + i);
		sndPar[i].label = txtLabels[MODE_SND_VOLUME + i];
		sndPar[i].set = setNothing;
	}

	chan = eeprom_read_byte((uint8_t*)EEPROM_INPUT);

	sndPar[MODE_SND_VOLUME].set = setVolume;
	sndPar[MODE_SND_BASS].set = setBass;
	sndPar[MODE_SND_MIDDLE].set = setMiddle;
	sndPar[MODE_SND_TREBLE].set = setTreble;
	sndPar[MODE_SND_PREAMP].set = setPreamp;
	sndPar[MODE_SND_BALANCE].set = setBalance;

	for (i = 0; i < CHAN_CNT; i++)
		sndPar[MODE_SND_GAIN0 + i].set = setGain;

	return;
}

void setAudioParams(void)
{
	muteVolume();
	setChan(chan);
	setBass(sndPar[MODE_SND_BASS].value);
	setPreamp(sndPar[MODE_SND_PREAMP].value);
	setMiddle(sndPar[MODE_SND_MIDDLE].value);
	setTreble(sndPar[MODE_SND_TREBLE].value);
	unmuteVolume();

	return;
}

void saveAudioParams(void)
{
	uint8_t i;

	for (i = 0; i < MODE_SND_END; i++)
		eeprom_update_byte((uint8_t*)EEPROM_VOLUME + i, sndPar[i].value);

	eeprom_update_byte((uint8_t*)EEPROM_INPUT, chan);

	return;
}
