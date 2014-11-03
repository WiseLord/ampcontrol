#include "tda7318.h"

#include <avr/eeprom.h>

#include "../i2c.h"
#include "../eeprom.h"
#include "../pins.h"
#include "../display/icons.h"

static uint8_t chan;
static uint8_t mute;

static sndParam sndPar[SND_PARAM_COUNT] = {
	{0x00, 0xC1, 0x00, 0x0A},	/* Volume */
	{0x00, 0xF9, 0x07, 0x10},	/* Bass */
	{0x00, 0xF9, 0x07, 0x10},	/* Treble */
	{0x00, 0xF1, 0x0F, 0x0A},	/* Front/Rear */
	{0x00, 0xF1, 0x0F, 0x0A},	/* Balance */
	{0x00, 0x00, 0x03, 0x32},	/* Gain 0 */
	{0x00, 0x00, 0x03, 0x32},	/* Gain 1 */
	{0x00, 0x00, 0x03, 0x32},	/* Gain 2 */
	{0x00, 0x00, 0x03, 0x32},	/* Gain 3 */
};


static void setVolume(int8_t val)
{
	int8_t spFrontLeft = 0;
	int8_t spFrontRight = 0;

	int8_t spRearLeft = 0;
	int8_t spRearRight = 0;

	if (sndPar[SND_BALANCE].value > 0) {
		spFrontRight -= sndPar[SND_BALANCE].value;
		spRearRight -= sndPar[SND_BALANCE].value;
	} else {
		spFrontLeft += sndPar[SND_BALANCE].value;
		spRearLeft += sndPar[SND_BALANCE].value;
	}
	if (sndPar[SND_FRONTREAR].value > 0) {
		spRearLeft -= sndPar[SND_FRONTREAR].value;
		spRearRight -= sndPar[SND_FRONTREAR].value;
	} else {
		spFrontLeft += sndPar[SND_FRONTREAR].value;
		spFrontRight += sndPar[SND_FRONTREAR].value;
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

static void setBalanceFrontRear(int8_t val)
{
	setVolume(sndPar[SND_VOLUME].value);

	return;
}

static void setBalance(int8_t val)
{
	setVolume(sndPar[SND_VOLUME].value);

	return;
}

static void setSwitch(int8_t gain)
{
	I2CStart(TDA7318_ADDR);
	I2CWriteByte(TDA7318_SW | (CHAN_CNT - gain) << 3 | chan);
	I2CStop();

	return;
}

static void setGain(int8_t val)
{
	setSwitch(val);

	return;
}


sndParam *sndParAddr(uint8_t index)
{
	return &sndPar[index];
}


uint8_t getChan(void)
{
	return chan;
}

uint8_t getMute(void)
{
	return mute;
}


void changeParam(sndParam *param, int8_t diff)
{
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
	setGain(sndPar[SND_GAIN0 + ch].value);
	setSwitch(sndPar[SND_GAIN0 + chan].value);

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
	setVolume(sndPar[SND_VOLUME].min);
	mute = MUTE_ON;
	PORT(STMU_MUTE) &= ~STMU_MUTE_LINE;

	return;
}

void unmuteVolume(void)
{
	setVolume(sndPar[SND_VOLUME].value);
	mute = MUTE_OFF;
	PORT(STMU_MUTE) |= STMU_MUTE_LINE;

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


void loadAudioParams(uint8_t **txtLabels)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++)
		sndPar[i].value = eeprom_read_byte(eepromVolume + i);

	sndPar[SND_VOLUME].label = txtLabels[LABEL_VOLUME];
	sndPar[SND_BASS].label = txtLabels[LABEL_BASS];
	sndPar[SND_TREBLE].label = txtLabels[LABEL_TREBLE];
	sndPar[SND_FRONTREAR].label = txtLabels[LABEL_FRONTREAR];
	sndPar[SND_BALANCE].label = txtLabels[LABEL_BALANCE];
	sndPar[SND_GAIN0].label = txtLabels[LABEL_GAIN0];
	sndPar[SND_GAIN1].label = txtLabels[LABEL_GAIN1];
	sndPar[SND_GAIN2].label = txtLabels[LABEL_GAIN2];
	sndPar[SND_GAIN3].label = txtLabels[LABEL_GAIN3];

	sndPar[SND_VOLUME].icon = icons_24_volume;
	sndPar[SND_BASS].icon = icons_24_bass;
	sndPar[SND_TREBLE].icon = icons_24_treble;
	sndPar[SND_FRONTREAR].icon = icons_24_frontrear;
	sndPar[SND_BALANCE].icon = icons_24_balance;
	sndPar[SND_GAIN0].icon = icons_24_tuner;
	sndPar[SND_GAIN1].icon = icons_24_pc;
	sndPar[SND_GAIN2].icon = icons_24_tv;
	sndPar[SND_GAIN3].icon = icons_24_dvd;

	chan = eeprom_read_byte(eepromChannel);

	sndPar[SND_VOLUME].set = setVolume;
	sndPar[SND_BASS].set = setBass;
	sndPar[SND_TREBLE].set = setTreble;
	sndPar[SND_FRONTREAR].set = setBalanceFrontRear;
	sndPar[SND_BALANCE].set = setBalance;

	for (i = 0; i < CHAN_CNT; i++) {
		sndPar[SND_GAIN0 + i].set = setGain;
	}

	muteVolume();
	setChan(chan);
	setBass(sndPar[SND_BASS].value);
	setBalanceFrontRear(0);
	setTreble(sndPar[SND_TREBLE].value);

	return;
}

void saveAudioParams(void)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++)
		eeprom_update_byte(eepromVolume + i, sndPar[i].value);

	eeprom_update_byte(eepromChannel, chan);

	return;
}
