#include "audio.h"

#include <avr/eeprom.h>

#include "i2c.h"

#include "eeprom.h"

sndParam *params[10] = {
	&volume,
	&bass,
	&middle,
	&treble,
	&preamp,
	&balance,
	&gain[0],
	&gain[1],
	&gain[2],
	&gain[3],
};


void setVolume(int8_t val)
{
	int8_t spFrontLeft = 0;
	int8_t spFrontRight = 0;

#ifdef TDA7439
	spFrontLeft = val;
	spFrontRight = val;

	if (balance.value > 0) {
		spFrontLeft -= balance.value;
		if (spFrontLeft < volume.min)
			spFrontLeft = volume.min;
	} else {
		spFrontRight += balance.value;
		if (spFrontRight < volume.min)
			spFrontRight = volume.min;
	}
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
	I2CWriteByte(-spFrontRight);
	I2CWriteByte(-spFrontLeft);
	I2CStop();
#else
	int8_t spRearLeft = 0;
	int8_t spRearRight = 0;

	if (balance.value > 0) {
		spFrontRight -= balance.value;
		spRearRight -= balance.value;
	} else {
		spFrontLeft += balance.value;
		spRearLeft += balance.value;
	}
	if (preamp.value > 0) {
		spRearLeft -= preamp.value;
		spRearRight -= preamp.value;
	} else {
		spFrontLeft += preamp.value;
		spFrontRight += preamp.value;
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

void setPreamp(int8_t val) /* For TDA7313 used as balance front/rear */
{
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_PREAMP);
	I2CWriteByte(-val);
	I2CStop();
#else
	setVolume(volume.value);
#endif
}

int8_t setBMT(int8_t val)
{
	if (val > 0)
		return 15 - val;
	return 7 + val;
}

void setBass(int8_t val)
{
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_BASS);
	I2CWriteByte(setBMT(val));
	I2CStop();
#else
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7313_BASS | setBMT(val));
	I2CStop();
#endif
}

void setMiddle(int8_t val)
{
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_MIDDLE);
	I2CWriteByte(setBMT(val));
	I2CStop();
#endif
}

void setTreble(int8_t val)
{
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_TREBLE);
	I2CWriteByte(setBMT(val));
	I2CStop();
#else
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7313_TREBLE | setBMT(val));
	I2CStop();
#endif
}

#ifndef TDA7439
void setSwitch(int8_t gain)
{
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7313_SW | (3 - gain) << 3 | loud << 2 | chan);
	I2CStop();
}
#endif

void setGain(int8_t val)
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
	setGain(gain[ch].value);
#ifdef TDA7439
	I2CStart(AUDIOPROC_ADDR);
	I2CWriteByte(TDA7439_INPUT_SELECT);
	I2CWriteByte(CHAN_CNT - 1 - ch);
	I2CStop();
#else
	setSwitch(gain[chan].value);
#endif
}

void setBalance(int8_t val)
{
	setVolume(volume.value);
}

void muteVolume(void)
{
	setVolume(volume.min);
	mute = MUTE_ON;
}

void unmuteVolume(void)
{
	setVolume(volume.value);
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
	if (loud == LOUDNESS_ON)
		loud = LOUDNESS_OFF;
	else
		loud = LOUDNESS_ON;
	setSwitch(gain[chan].value);
}
#endif

void loadParams(uint8_t **txtLabels)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++) {
		params[i]->value = eeprom_read_byte(eepromVolume + i);
		params[i]->label = txtLabels[i];
		params[i]->min = eeprom_read_byte(eepromMinimums + i);
		params[i]->max = eeprom_read_byte(eepromMaximums + i);
		params[i]->step = eeprom_read_byte(eepromSteps + i);
	}

	chan = eeprom_read_byte(eepromChannel);
	loud = eeprom_read_byte(eepromLoudness);

	volume.set = setVolume;
	bass.set = setBass;
	middle.set = setMiddle;
	treble.set = setTreble;
	balance.set = setBalance;
	preamp.set = setPreamp;

	for (i = 0; i < 4; i++) {
		gain[i].set = setGain;
	}

	setChan(chan);
	setPreamp(preamp.value);
	setBass(bass.value);
	setMiddle(middle.value);
	setTreble(treble.value);
}

void saveAudioParams(void)
{
	uint8_t i;

	for (i = 0; i < SND_PARAM_COUNT; i++) {
		eeprom_write_byte(eepromVolume + i, params[i]->value);
	}
	eeprom_write_byte(eepromLoudness, loud);
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

void nextChan(void)
{
	chan++;
	if (chan >= CHAN_CNT)
		chan = 0;
	setChan(chan);
}
