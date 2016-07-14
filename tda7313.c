#include "tda7313.h"

#include <avr/eeprom.h>

#include "i2c.h"
#include "eeprom.h"

static uint8_t chan;
static uint8_t mute;
static uint8_t loud;

static sndParam sndPar[SND_PARAM_COUNT] = {
	{0x00, 0xC1, 0x00, 0x0A},	/* Volume */
	{0x00, 0xF9, 0x07, 0x10},	/* Bass */
	{0x00, 0xF9, 0x07, 0x10},	/* Treble */
	{0x00, 0xF1, 0x0F, 0x0A},	/* Front/Rear */
	{0x00, 0xF1, 0x0F, 0x0A},	/* Balance */
	{0x00, 0x00, 0x03, 0x1E},	/* Gain 0 */
	{0x00, 0x00, 0x03, 0x1E},	/* Gain 1 */
	{0x00, 0x00, 0x03, 0x1E},	/* Gain 2 */
};


static void setVolume(int8_t val)
{
	int8_t spFrontLeft = 0;
	int8_t spFrontRight = 0;
	int8_t spRearLeft = 0;
	int8_t spRearRight = 0;

	if (sndPar[SND_BALANCE].value > 0) {
		spFrontRight += sndPar[SND_BALANCE].value;
		spRearRight += sndPar[SND_BALANCE].value;
	} else {
		spFrontLeft -= sndPar[SND_BALANCE].value;
		spRearLeft -= sndPar[SND_BALANCE].value;
	}
	if (sndPar[SND_FRONTREAR].value > 0) {
		spRearLeft += sndPar[SND_FRONTREAR].value;
		spRearRight += sndPar[SND_FRONTREAR].value;
	} else {
		spFrontLeft -= sndPar[SND_FRONTREAR].value;
		spFrontRight -= sndPar[SND_FRONTREAR].value;
	}

	I2CStart(TDA7313_ADDR);
	I2CWriteByte(TDA7313_VOLUME | -val);
	if (mute == MUTE_ON) {
		I2CWriteByte(TDA7313_SP_FRONT_LEFT | MUTE_VAL);
		I2CWriteByte(TDA7313_SP_FRONT_RIGHT | MUTE_VAL);
		I2CWriteByte(TDA7313_SP_REAR_LEFT | MUTE_VAL);
		I2CWriteByte(TDA7313_SP_REAR_RIGHT | MUTE_VAL);
	} else {
		I2CWriteByte(TDA7313_SP_FRONT_LEFT | spFrontLeft);
		I2CWriteByte(TDA7313_SP_FRONT_RIGHT | spFrontRight);
		I2CWriteByte(TDA7313_SP_REAR_LEFT | spRearLeft);
		I2CWriteByte(TDA7313_SP_REAR_RIGHT | spRearRight);
	}
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
	I2CStart(TDA7313_ADDR);
	I2CWriteByte(TDA7313_BASS | calcBMT(val));
	I2CStop();

	return;
}

static void setTreble(int8_t val)
{
	I2CStart(TDA7313_ADDR);
	I2CWriteByte(TDA7313_TREBLE | calcBMT(val));
	I2CStop();

	return;
}

static void setBalance(int8_t val)
{
	setVolume(sndPar[SND_VOLUME].value);

	return;
}

static void setGain(int8_t gain)
{
	I2CStart(TDA7313_ADDR);
	I2CWriteByte(TDA7313_SW | ((CHAN_CNT - gain) << 3) | (loud << 2) | chan);
	I2CStop();

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

uint8_t getLoudness(void)
{
	return !loud;
}


void changeParam(uint8_t index, int8_t diff)
{
	sndParam *param = &sndPar[index];

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
	setGain(sndPar[SND_GAIN0 + chan].value);

	// Set low level on all inputs
	PORT(IN0) &= ~IN0_LINE;
	PORT(IN1) &= ~IN1_LINE;
	PORT(IN2) &= ~IN2_LINE;

	// Set high level on selected input control
	switch (chan) {
	case 1:
		PORT(IN1) |= IN1_LINE;
		break;
	case 2:
		PORT(IN2) |= IN2_LINE;
		break;
	default:
		PORT(IN0) |= IN0_LINE;
		break;
	}

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
	mute = MUTE_ON;
	setVolume(sndPar[SND_VOLUME].min);

	return;
}

void unmuteVolume(void)
{
	mute = MUTE_OFF;
	setVolume(sndPar[SND_VOLUME].value);

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
	loud = !loud;
	setGain(sndPar[SND_GAIN0 + chan].value);

	return;
}


void loadAudioParams(uint8_t **txtLabels)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++) {
		sndPar[i].value = eeprom_read_byte(eepromVolume + i);
		sndPar[i].label = txtLabels[i];
	}

	chan = eeprom_read_byte(eepromChannel);
	loud = eeprom_read_byte(eepromLoudness);

	sndPar[SND_VOLUME].set = setVolume;
	sndPar[SND_BASS].set = setBass;
	sndPar[SND_TREBLE].set = setTreble;
	sndPar[SND_BALANCE].set = setBalance;
	sndPar[SND_FRONTREAR].set = setBalance;

	for (i = 0; i < CHAN_CNT; i++)
		sndPar[SND_GAIN0 + i].set = setGain;

	return;
}

void setAudioParams(void)
{
	muteVolume();
	setChan(chan);
	setBass(sndPar[SND_BASS].value);
	setTreble(sndPar[SND_TREBLE].value);
	setBalance(0);
	unmuteVolume();

	return;
}

void saveAudioParams(void)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++)
		eeprom_update_byte(eepromVolume + i, sndPar[i].value);

	eeprom_update_byte(eepromLoudness, loud);
	eeprom_update_byte(eepromChannel, chan);

	return;
}
