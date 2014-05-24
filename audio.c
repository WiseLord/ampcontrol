#include "audio.h"

#include <avr/eeprom.h>

#include "i2c.h"
#include "eeprom.h"

static sndParam sndPar[SND_PARAM_COUNT];

static uint8_t chan;
static uint8_t mute;
#ifdef TDA7313
static uint8_t loud;
#endif

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

#ifdef TDA7313
uint8_t getLoudness()
{
	return loud;
}
#endif

static void setVolume(int8_t val)
{
	int8_t spFrontLeft = 0;
	int8_t spFrontRight = 0;

#ifdef TDA7439
	spFrontLeft = val;
	spFrontRight = val;

	if (sndPar[SND_BALANCE].value > 0) {
		spFrontLeft -= sndPar[SND_BALANCE].value;
		if (spFrontLeft < sndPar[SND_VOLUME].min)
			spFrontLeft = sndPar[SND_VOLUME].min;
	} else {
		spFrontRight += sndPar[SND_BALANCE].value;
		if (spFrontRight < sndPar[SND_VOLUME].min)
			spFrontRight = sndPar[SND_VOLUME].min;
	}
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
	I2CWriteByte(-spFrontRight);
	I2CWriteByte(-spFrontLeft);
	I2CStop();
#else
	int8_t spRearLeft = 0;
	int8_t spRearRight = 0;

	if (sndPar[SND_BALANCE].value > 0) {
		spFrontRight -= sndPar[SND_BALANCE].value;
		spRearRight -= sndPar[SND_BALANCE].value;
	} else {
		spFrontLeft += sndPar[SND_BALANCE].value;
		spRearLeft += sndPar[SND_BALANCE].value;
	}
	if (sndPar[SND_BALANCE_FR].value > 0) {
		spRearLeft -= sndPar[SND_BALANCE_FR].value;
		spRearRight -= sndPar[SND_BALANCE_FR].value;
	} else {
		spFrontLeft += sndPar[SND_BALANCE_FR].value;
		spFrontRight += sndPar[SND_BALANCE_FR].value;
	}
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7313_VOLUME | -val);
	I2CWriteByte(TDA7313_SP_FRONT_LEFT | -spFrontLeft);
	I2CWriteByte(TDA7313_SP_FRONT_RIGHT | -spFrontRight);
	I2CWriteByte(TDA7313_SP_REAR_LEFT | -spRearLeft);
	I2CWriteByte(TDA7313_SP_REAR_RIGHT | -spRearRight);
	I2CStop();
#endif
}

#ifdef TDA7439
static void setPreamp(int8_t val)
{
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_PREAMP);
	I2CWriteByte(-val);
	I2CStop();
}
#endif

#if defined(TDA7313) || defined(TDA7318)
static void setBalanceFrontRear(int8_t val)
{
	setVolume(sndPar[SND_VOLUME].value);
}
#endif

static int8_t calcBMT(int8_t val)
{
	if (val > 0)
		return 15 - val;
	return 7 + val;
}

static void setBass(int8_t val)
{
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_BASS);
	I2CWriteByte(calcBMT(val));
	I2CStop();
#else
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7313_BASS | calcBMT(val));
	I2CStop();
#endif
}

#ifdef TDA7439
static void setMiddle(int8_t val)
{
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_MIDDLE);
	I2CWriteByte(calcBMT(val));
	I2CStop();
}
#endif

static void setTreble(int8_t val)
{
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_TREBLE);
	I2CWriteByte(calcBMT(val));
	I2CStop();
#else
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7313_TREBLE | calcBMT(val));
	I2CStop();
#endif
}

#if defined(TDA7313) || defined(TDA7318)
static void setSwitch(int8_t gain)
{
	I2CStart(AUDIOPROC_ADDR);
#ifdef TDA7313
	I2CWriteByte(TDA7313_SW | (CHAN_CNT - gain) << 3 | loud << 2 | chan);
#else
	I2CWriteByte(TDA7313_SW | (CHAN_CNT - gain) << 3 | chan);
#endif
	I2CStop();
}
#endif

static void setGain(int8_t val)
{
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_INPUT_GAIN);
	I2CWriteByte(val);
	I2CStop();
#else
	setSwitch(val);
#endif
}

void setChan(uint8_t ch)
{
	chan = ch;
	setGain(sndPar[SND_GAIN0 + ch].value);
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_INPUT_SELECT);
	I2CWriteByte(CHAN_CNT - 1 - ch);
	I2CStop();
#else
	setSwitch(sndPar[SND_GAIN0 + chan].value);
#endif
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
}

void unmuteVolume(void)
{
	setVolume(sndPar[SND_VOLUME].value);
	mute = MUTE_OFF;
}

void switchMute(void)
{
	if (mute == MUTE_ON) {
		unmuteVolume();
	} else {
		muteVolume();
	}
}

#ifdef TDA7313
void switchLoudness(void)
{
	loud = !loud;
	setSwitch(sndPar[SND_GAIN0 + chan].value);
}
#endif

void loadAudioParams(uint8_t **txtLabels)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++) {
		sndPar[i].value = eeprom_read_byte(eepromVolume + i);
		sndPar[i].label = txtLabels[i];
		sndPar[i].min = eeprom_read_byte(eepromMinimums + i);
		sndPar[i].max = eeprom_read_byte(eepromMaximums + i);
		sndPar[i].step = eeprom_read_byte(eepromSteps + i);
	}

	chan = eeprom_read_byte(eepromChannel);
#ifdef TDA7313
	loud = eeprom_read_byte(eepromLoudness);
#endif

	sndPar[SND_VOLUME].set = setVolume;
	sndPar[SND_BASS].set = setBass;
#if defined(TDA7439)
	sndPar[SND_MIDDLE].set = setMiddle;
#endif
	sndPar[SND_TREBLE].set = setTreble;
#if defined(TDA7439)
	sndPar[SND_PREAMP].set = setPreamp;
#else
	sndPar[SND_BALANCE_FR].set = setBalanceFrontRear;
#endif
	sndPar[SND_BALANCE].set = setBalance;

	for (i = 0; i < 4; i++) {
		sndPar[SND_GAIN0 + i].set = setGain;
	}

	setChan(chan);
	setBass(sndPar[SND_BASS].value);
#ifdef TDA7439
	setPreamp(sndPar[SND_PREAMP].value);
	setMiddle(sndPar[SND_MIDDLE].value);
#else
	setBalanceFrontRear(0);
#endif
	setTreble(sndPar[SND_TREBLE].value);
}

void saveAudioParams(void)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++) {
		eeprom_write_byte(eepromVolume + i, sndPar[i].value);
	}
#ifdef TDA7313
	eeprom_write_byte(eepromLoudness, loud);
#endif
	eeprom_write_byte(eepromChannel, chan);
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
