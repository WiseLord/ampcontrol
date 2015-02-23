#include "audioproc.h"

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "../eeprom.h"
#include "../display/icons.h"

static uint8_t chan;
static uint8_t mute;

static const sndGrid grid[MODE_SND_END] PROGMEM = {
	{-79,  0, 1 * 8},	/* Volume */
	{ -7,  7, 2 * 8},	/* Bass */
	{ -7,  7, 2 * 8},	/* Middle */
	{ -7,  7, 2 * 8},	/* Treble */
	{-47,  0, 1 * 8},	/* Preamp */
	{  0,  0, 0 * 8},	/* FrontRear */
	{-21, 21, 1 * 8},	/* Balance */
	{  0, 15, 2 * 8},	/* Gain 0 */
	{  0, 15, 2 * 8},	/* Gain 1 */
	{  0, 15, 2 * 8},	/* Gain 2 */
	{  0, 15, 2 * 8},	/* Gain 3 */
};
static sndParam sndPar[MODE_SND_END];


static void setVolume(int8_t val)
{

	return;
}

static void setBass(int8_t val)
{

	return;
}

static void setMiddle(int8_t val)
{

	return;
}

static void setTreble(int8_t val)
{

	return;
}

static void setPreamp(int8_t val)
{

	return;
}

static void setFrontRear(int8_t val)
{

	return;
}

static void setBalance(int8_t val)
{

	return;
}

static void setGain(int8_t val)
{

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


void changeParam(uint8_t dispMode, int8_t diff)
{
	sndParam *param = sndParAddr(dispMode);
	param->value += diff;
	if (param->value > (int8_t)pgm_read_byte(&param->grid->max))
		param->value = (int8_t)pgm_read_byte(&param->grid->max);
	if (param->value < (int8_t)pgm_read_byte(&param->grid->min))
		param->value = (int8_t)pgm_read_byte(&param->grid->min);
	param->set(param->value);

	return;
}


void setChan(uint8_t ch)
{
	chan = ch;

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
	setVolume((int8_t)pgm_read_byte(&sndPar[MODE_SND_VOLUME].grid->min));
	mute = MUTE_ON;
//	PORT(STMU_MUTE) &= ~STMU_MUTE_LINE;

	return;
}

void unmuteVolume(void)
{
	setVolume(sndPar[MODE_SND_VOLUME].value);
	mute = MUTE_OFF;
//	PORT(STMU_MUTE) |= STMU_MUTE_LINE;

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

	for (i = 0; i < MODE_SND_END; i++)
		sndPar[i].grid = &grid[i];

	for (i = 0; i < MODE_SND_END; i++)
		sndPar[i].value = eeprom_read_byte(eepromVolume + i);

	sndPar[MODE_SND_VOLUME].label = txtLabels[LABEL_VOLUME];
	sndPar[MODE_SND_BASS].label = txtLabels[LABEL_BASS];
	sndPar[MODE_SND_MIDDLE].label = txtLabels[LABEL_MIDDLE];
	sndPar[MODE_SND_TREBLE].label = txtLabels[LABEL_TREBLE];
	sndPar[MODE_SND_PREAMP].label = txtLabels[LABEL_PREAMP];
	sndPar[MODE_SND_FRONTREAR].label = txtLabels[LABEL_FRONTREAR];
	sndPar[MODE_SND_BALANCE].label = txtLabels[LABEL_BALANCE];
	sndPar[MODE_SND_GAIN0].label = txtLabels[LABEL_GAIN0];
	sndPar[MODE_SND_GAIN1].label = txtLabels[LABEL_GAIN1];
	sndPar[MODE_SND_GAIN2].label = txtLabels[LABEL_GAIN2];
	sndPar[MODE_SND_GAIN3].label = txtLabels[LABEL_GAIN3];

	sndPar[MODE_SND_VOLUME].icon = icons_24_volume;
	sndPar[MODE_SND_BASS].icon = icons_24_bass;
	sndPar[MODE_SND_MIDDLE].icon = icons_24_middle;
	sndPar[MODE_SND_TREBLE].icon = icons_24_treble;
	sndPar[MODE_SND_PREAMP].icon = icons_24_preamp;
	sndPar[MODE_SND_FRONTREAR].icon = icons_24_frontrear;
	sndPar[MODE_SND_BALANCE].icon = icons_24_balance;
	sndPar[MODE_SND_GAIN0].icon = icons_24_tuner;
	sndPar[MODE_SND_GAIN1].icon = icons_24_pc;
	sndPar[MODE_SND_GAIN2].icon = icons_24_tv;
	sndPar[MODE_SND_GAIN3].icon = icons_24_dvd;

	chan = eeprom_read_byte(eepromChannel);

	sndPar[MODE_SND_VOLUME].set = setVolume;
	sndPar[MODE_SND_BASS].set = setBass;
	sndPar[MODE_SND_MIDDLE].set = setMiddle;
	sndPar[MODE_SND_TREBLE].set = setTreble;
	sndPar[MODE_SND_PREAMP].set = setPreamp;
	sndPar[MODE_SND_FRONTREAR].set = setFrontRear;
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
		eeprom_update_byte(eepromVolume + i, sndPar[i].value);

	eeprom_update_byte(eepromChannel, chan);

	return;
}

uint8_t getLoudness()
{
	return 0;
}
