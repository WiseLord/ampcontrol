#include "tda7439.h"

#include <avr/eeprom.h>

#include "../i2c.h"
#include "../eeprom.h"
#include "../input.h"

static sndParam sndPar[SND_PARAM_COUNT] = {
	{0x00, 0xB1, 0x00, 0x08, (void*)0, (void*)0},	/* Volume */
	{0x00, 0xF9, 0x07, 0x10, (void*)0, (void*)0},	/* Bass */
	{0x00, 0xF9, 0x07, 0x10, (void*)0, (void*)0},	/* Middle */
	{0x00, 0xF9, 0x07, 0x10, (void*)0, (void*)0},	/* Treble */
	{0x00, 0xD1, 0x00, 0x08, (void*)0, (void*)0},	/* Preamp */
	{0x00, 0xEB, 0x15, 0x08, (void*)0, (void*)0},	/* Balance */
	{0x00, 0x00, 0x0F, 0x10, (void*)0, (void*)0},	/* Gain 0 */
	{0x00, 0x00, 0x0F, 0x10, (void*)0, (void*)0},	/* Gain 1 */
	{0x00, 0x00, 0x0F, 0x10, (void*)0, (void*)0},	/* Gain 2 */
	{0x00, 0x00, 0x0F, 0x10, (void*)0, (void*)0},	/* Gain 3 */
};

static uint8_t chan;
static uint8_t mute;

sndParam *sndParAddr(uint8_t index)
{
	return &sndPar[index];
}

uint8_t getChan()
{
	return chan;
}

uint8_t getMute()
{
	return mute;
}

static void setVolume(int8_t val)
{
	int8_t spLeft = val;
	int8_t spRight = val;

	if (sndPar[SND_BALANCE].value > 0) {
		spLeft -= sndPar[SND_BALANCE].value;
		if (spLeft < sndPar[SND_VOLUME].min)
			spLeft = sndPar[SND_VOLUME].min;
	} else {
		spRight += sndPar[SND_BALANCE].value;
		if (spRight < sndPar[SND_VOLUME].min)
			spRight = sndPar[SND_VOLUME].min;
	}
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
	I2CWriteByte(-spRight);
	I2CWriteByte(-spLeft);
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

static void setGain(int8_t val)
{
	I2CStart(TDA7439_ADDR);
	I2CWriteByte(TDA7439_INPUT_GAIN);
	I2CWriteByte(val);
	I2CStop();

	return;
}

void setChan(uint8_t ch)
{
	chan = ch;
	setGain(sndPar[SND_GAIN0 + ch].value);

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
}

static void setBalance(int8_t val)
{
	setVolume(sndPar[SND_VOLUME].value);
}

void muteVolume(void)
{
	setVolume(sndPar[SND_VOLUME].min);
	mute = MUTE_ON;
	STMU_PORT &= ~MUTE;
}

void unmuteVolume(void)
{
	setVolume(sndPar[SND_VOLUME].value);
	mute = MUTE_OFF;
	STMU_PORT |= MUTE;
}

void switchMute(void)
{
	if (mute == MUTE_ON) {
		unmuteVolume();
	} else {
		muteVolume();
	}
}

void loadAudioParams(uint8_t **txtLabels)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++) {
		sndPar[i].value = eeprom_read_byte(eepromVolume + i);
		sndPar[i].label = txtLabels[i];
	}

	chan = eeprom_read_byte(eepromChannel);

	sndPar[SND_VOLUME].set = setVolume;
	sndPar[SND_BASS].set = setBass;
	sndPar[SND_MIDDLE].set = setMiddle;
	sndPar[SND_TREBLE].set = setTreble;
	sndPar[SND_PREAMP].set = setPreamp;
	sndPar[SND_BALANCE].set = setBalance;

	for (i = 0; i < 4; i++) {
		sndPar[SND_GAIN0 + i].set = setGain;
	}

	muteVolume();
	setChan(chan);
	setBass(sndPar[SND_BASS].value);
	setPreamp(sndPar[SND_PREAMP].value);
	setMiddle(sndPar[SND_MIDDLE].value);
	setTreble(sndPar[SND_TREBLE].value);
}

void saveAudioParams(void)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++) {
		eeprom_update_byte(eepromVolume + i, sndPar[i].value);
	}
	eeprom_update_byte(eepromChannel, chan);
}

void changeParam(sndParam *param, int8_t diff)
{
	param->value += diff;
	if (param->value > param->max)
		param->value = param->max;
	if (param->value < param->min)
		param->value = param->min;
	param->set(param->value);
}
