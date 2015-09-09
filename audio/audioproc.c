#include "audioproc.h"

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "../eeprom.h"
#include "../display.h"
#include "../pins.h"

static const sndGrid grid[] PROGMEM = {
	{  0,  0, 0.00 * 8},	/* 0: Not implemented */
	{-79,  0, 1.00 * 8},	/* 1: -79..0dB with 1dB step */
	{ -7,  7, 2.00 * 8},	/* 2: -14..14dB with 2dB step */
	{-47,  0, 1.00 * 8},	/* 3: -47..0dB with 1dB step */
	{-21, 21, 1.00 * 8},	/* 4: -21..21dB with 1dB step */
	{  0, 15, 2.00 * 8},	/* 5: 0..30dB with 2dB step */
	{-63,  0, 1.25 * 8},	/* 6: -78.75..0dB with 1.25dB step*/
	{-15, 15, 1.25 * 8},	/* 7: -18.75..18.75dB with 1.25dB step */
	{  0,  3, 3.75 * 8},	/* 8: 0..11.25dB with 3.75dB step */
	{  0,  3, 6.25 * 8},	/* 9: 0..18.75dB with 6.25dB step */
	{-21,  0, 1.00 * 8},	/*10: -21..0dB with 1dB step */
	{-96, 31, 1.00 * 8},	/*11: -96..31dB with 1dB step */
};

static sndParam sndPar[MODE_SND_END];
static audioProc _aproc;

static uint8_t _inCnt;
static uint8_t _input;
static uint8_t _mute;
static uint8_t _loudness;
static uint8_t usePGA2310;

static void setNothing(int8_t val)
{
	return;
}

void sndInit(uint8_t extFunc)
{
	uint8_t i;

	uint8_t **txtLabels = getTxtLabels();

	/* Load audio parameters stored in eeprom */
	for (i = 0; i < MODE_SND_END; i++) {
		sndPar[i].value = eeprom_read_byte((uint8_t*)EEPROM_VOLUME + i);
		sndPar[i].label = txtLabels[MODE_SND_VOLUME + i];
	}
	_input = eeprom_read_byte((uint8_t*)EEPROM_INPUT);
	_loudness = eeprom_read_byte((uint8_t*)EEPROM_LOUDNESS);
	_aproc = eeprom_read_byte((uint8_t*)EEPROM_AUDIOPROC);
	if (_aproc >= AUDIOPROC_END)
		_aproc = AUDIOPROC_TDA7439;

	switch (_aproc) {
	case AUDIOPROC_TDA7439:
		tda7439Init(sndPar);
		break;
	case AUDIOPROC_TDA7312:
	case AUDIOPROC_TDA7313:
	case AUDIOPROC_TDA7314:
	case AUDIOPROC_TDA7315:
	case AUDIOPROC_TDA7318:
	case AUDIOPROC_PT2314:
		tda731xInit(sndPar);
		break;
	case AUDIOPROC_TDA7448:
		tda7448Init(sndPar);
		break;
	case AUDIOPROC_PGA2310:
		if (usePGA2310)
			pga2310Init(sndPar);
		break;
	default:
		break;
	}

	/* Init grid and functions with empty values */
	for (i = 0; i < MODE_SND_END; i++) {
		sndPar[i].grid = &grid[0];
		sndPar[i].set = setNothing;
	}

	/* Setup audio parameters grid and functions */
	switch (_aproc) {
	case AUDIOPROC_TDA7439:
		sndPar[MODE_SND_VOLUME].grid = &grid[1];
		sndPar[MODE_SND_BASS].grid = &grid[2];
		sndPar[MODE_SND_MIDDLE].grid = &grid[2];
		sndPar[MODE_SND_TREBLE].grid = &grid[2];
		sndPar[MODE_SND_PREAMP].grid = &grid[3];
		sndPar[MODE_SND_BALANCE].grid = &grid[4];
		sndPar[MODE_SND_GAIN0].grid = &grid[5];
		sndPar[MODE_SND_GAIN1].grid = &grid[5];
		sndPar[MODE_SND_GAIN2].grid = &grid[5];
		sndPar[MODE_SND_GAIN3].grid = &grid[5];
		_inCnt = TDA7439_IN_CNT;
		sndPar[MODE_SND_VOLUME].set = tda7439SetVolume;
		sndPar[MODE_SND_BASS].set = tda7439SetBass;
		sndPar[MODE_SND_MIDDLE].set = tda7439SetMiddle;
		sndPar[MODE_SND_TREBLE].set = tda7439SetTreble;
		sndPar[MODE_SND_PREAMP].set = tda7439SetPreamp;
		sndPar[MODE_SND_BALANCE].set= tda7439SetBalance;
		sndPar[MODE_SND_GAIN0].set = tda7439SetGain;
		sndPar[MODE_SND_GAIN1].set = tda7439SetGain;
		sndPar[MODE_SND_GAIN2].set = tda7439SetGain;
		sndPar[MODE_SND_GAIN3].set = tda7439SetGain;
		break;
	case AUDIOPROC_TDA7312:
		sndPar[MODE_SND_VOLUME].grid = &grid[6];
		sndPar[MODE_SND_BASS].grid = &grid[2];
		sndPar[MODE_SND_TREBLE].grid = &grid[2];
		sndPar[MODE_SND_BALANCE].grid = &grid[7];
		_inCnt = TDA7312_IN_CNT;
		sndPar[MODE_SND_VOLUME].set = tda731xSetVolume;
		sndPar[MODE_SND_BASS].set = tda731xSetBass;
		sndPar[MODE_SND_TREBLE].set = tda731xSetTreble;
		sndPar[MODE_SND_BALANCE].set = tda731xSetBalance;
		break;
	case AUDIOPROC_TDA7313:
		sndPar[MODE_SND_VOLUME].grid = &grid[6];
		sndPar[MODE_SND_BASS].grid = &grid[2];
		sndPar[MODE_SND_TREBLE].grid = &grid[2];
		sndPar[MODE_SND_FRONTREAR].grid = &grid[7];
		sndPar[MODE_SND_BALANCE].grid = &grid[7];
		sndPar[MODE_SND_GAIN0].grid = &grid[8];
		sndPar[MODE_SND_GAIN1].grid = &grid[8];
		sndPar[MODE_SND_GAIN2].grid = &grid[8];
		_inCnt = TDA7313_IN_CNT;
		sndPar[MODE_SND_VOLUME].set = tda731xSetVolume;
		sndPar[MODE_SND_BASS].set = tda731xSetBass;
		sndPar[MODE_SND_TREBLE].set = tda731xSetTreble;
		sndPar[MODE_SND_FRONTREAR].set = tda731xSetBalance;
		sndPar[MODE_SND_BALANCE].set = tda731xSetBalance;
		sndPar[MODE_SND_GAIN0].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN1].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN2].set = tda731xSetGain;
		break;
	case AUDIOPROC_TDA7314:
		sndPar[MODE_SND_VOLUME].grid = &grid[6];
		sndPar[MODE_SND_BASS].grid = &grid[2];
		sndPar[MODE_SND_TREBLE].grid = &grid[2];
		sndPar[MODE_SND_FRONTREAR].grid = &grid[7];
		sndPar[MODE_SND_BALANCE].grid = &grid[7];
		sndPar[MODE_SND_GAIN0].grid = &grid[9];
		_inCnt = TDA7314_IN_CNT;
		break;
		sndPar[MODE_SND_VOLUME].set = tda731xSetVolume;
		sndPar[MODE_SND_BASS].set = tda731xSetBass;
		sndPar[MODE_SND_TREBLE].set = tda731xSetTreble;
		sndPar[MODE_SND_FRONTREAR].set = tda731xSetBalance;
		sndPar[MODE_SND_BALANCE].set = tda731xSetBalance;
		sndPar[MODE_SND_GAIN0].set = tda731xSetGain;
	case AUDIOPROC_TDA7315:
		sndPar[MODE_SND_VOLUME].grid = &grid[6];
		sndPar[MODE_SND_BASS].grid = &grid[2];
		sndPar[MODE_SND_TREBLE].grid = &grid[2];
		sndPar[MODE_SND_BALANCE].grid = &grid[7];
		_inCnt = TDA7315_IN_CNT;
		break;
		sndPar[MODE_SND_VOLUME].set = tda731xSetVolume;
		sndPar[MODE_SND_BASS].set = tda731xSetBass;
		sndPar[MODE_SND_TREBLE].set = tda731xSetTreble;
		sndPar[MODE_SND_BALANCE].set = tda731xSetBalance;
	case AUDIOPROC_TDA7318:
		sndPar[MODE_SND_VOLUME].grid = &grid[6];
		sndPar[MODE_SND_BASS].grid = &grid[2];
		sndPar[MODE_SND_TREBLE].grid = &grid[2];
		sndPar[MODE_SND_FRONTREAR].grid = &grid[7];
		sndPar[MODE_SND_BALANCE].grid = &grid[7];
		sndPar[MODE_SND_GAIN0].grid = &grid[9];
		sndPar[MODE_SND_GAIN1].grid = &grid[9];
		sndPar[MODE_SND_GAIN2].grid = &grid[9];
		sndPar[MODE_SND_GAIN3].grid = &grid[9];
		_inCnt = TDA7318_IN_CNT;
		sndPar[MODE_SND_VOLUME].set = tda731xSetVolume;
		sndPar[MODE_SND_BASS].set = tda731xSetBass;
		sndPar[MODE_SND_TREBLE].set = tda731xSetTreble;
		sndPar[MODE_SND_FRONTREAR].set = tda731xSetBalance;
		sndPar[MODE_SND_BALANCE].set = tda731xSetBalance;
		sndPar[MODE_SND_GAIN0].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN1].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN2].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN3].set =tda731xSetGain ;
		break;
	case AUDIOPROC_PT2314:
		sndPar[MODE_SND_VOLUME].grid = &grid[6];
		sndPar[MODE_SND_BASS].grid = &grid[2];
		sndPar[MODE_SND_TREBLE].grid = &grid[2];
		sndPar[MODE_SND_FRONTREAR].grid = &grid[7];
		sndPar[MODE_SND_BALANCE].grid = &grid[7];
		sndPar[MODE_SND_GAIN0].grid = &grid[8];
		sndPar[MODE_SND_GAIN1].grid = &grid[8];
		sndPar[MODE_SND_GAIN2].grid = &grid[8];
		sndPar[MODE_SND_GAIN3].grid = &grid[8];
		_inCnt = PT2314_IN_CNT;
		sndPar[MODE_SND_VOLUME].set = tda731xSetVolume;
		sndPar[MODE_SND_BASS].set = tda731xSetBass;
		sndPar[MODE_SND_TREBLE].set = tda731xSetTreble;
		sndPar[MODE_SND_FRONTREAR].set = tda731xSetBalance;
		sndPar[MODE_SND_BALANCE].set = tda731xSetBalance;
		sndPar[MODE_SND_GAIN0].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN1].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN2].set = tda731xSetGain;
		sndPar[MODE_SND_GAIN3].set = tda731xSetGain;
		break;
	case AUDIOPROC_TDA7448:
		sndPar[MODE_SND_VOLUME].grid = &grid[1];
		sndPar[MODE_SND_FRONTREAR].grid = &grid[4];
		sndPar[MODE_SND_BALANCE].grid = &grid[4];
		sndPar[MODE_SND_CENTER].grid = &grid[10];
		sndPar[MODE_SND_SUBWOOFER].grid = &grid[10];
		_inCnt = TDA7448_IN_CNT;
		sndPar[MODE_SND_VOLUME].set = tda7448SetSpeakers;
		sndPar[MODE_SND_FRONTREAR].set = tda7448SetSpeakers;
		sndPar[MODE_SND_BALANCE].set= tda7448SetSpeakers;
		sndPar[MODE_SND_CENTER].set = tda7448SetSpeakers;
		sndPar[MODE_SND_SUBWOOFER].set = tda7448SetSpeakers;
		break;
	case AUDIOPROC_PGA2310:
		sndPar[MODE_SND_VOLUME].grid = &grid[11];
		sndPar[MODE_SND_BALANCE].grid = &grid[4];
		_inCnt = PGA2310_IN_CNT;
		sndPar[MODE_SND_VOLUME].set = pga2310SetSpeakers;
		sndPar[MODE_SND_BALANCE].set = pga2310SetSpeakers;
		break;
	default:
		break;
	}

#ifndef KS0066
	/* Setup icons for audio parameters */
	sndPar[MODE_SND_VOLUME].icon = icons_24_volume;
	sndPar[MODE_SND_BASS].icon = icons_24_bass;
	sndPar[MODE_SND_MIDDLE].icon = icons_24_middle;
	sndPar[MODE_SND_TREBLE].icon = icons_24_treble;
	sndPar[MODE_SND_PREAMP].icon = icons_24_preamp;
	sndPar[MODE_SND_FRONTREAR].icon = icons_24_frontrear;
	sndPar[MODE_SND_BALANCE].icon = icons_24_balance;
	sndPar[MODE_SND_CENTER].icon = icons_24_center;
	sndPar[MODE_SND_SUBWOOFER].icon = icons_24_subwoofer;
	sndPar[MODE_SND_GAIN0].icon = icons_24_tuner;
	sndPar[MODE_SND_GAIN1].icon = icons_24_pc;
	sndPar[MODE_SND_GAIN2].icon = icons_24_tv;
	sndPar[MODE_SND_GAIN3].icon = icons_24_dvd;
#endif
	return;
}

sndParam *sndParAddr(uint8_t index)
{
	return &sndPar[index];
}

uint8_t sndInputCnt(void)
{
	return _inCnt;
}

void sndSetInput(uint8_t input)
{
	if (input >= _inCnt)
		input = 0;
	_input = input;

	switch (_aproc) {
	case AUDIOPROC_TDA7439:
		tda7439SetInput(_input);
		break;
	case AUDIOPROC_TDA7312:
	case AUDIOPROC_TDA7313:
	case AUDIOPROC_TDA7314:
	case AUDIOPROC_TDA7315:
	case AUDIOPROC_TDA7318:
	case AUDIOPROC_PT2314:
		tda731xSetInput(_input);
		break;
	default:
		break;
	}
	return;
}

uint8_t sndGetInput(void)
{
	return _input;
}


void sndSetMute(uint8_t value)
{
	int8_t vol;

	if (value == MUTE_ON) {
		vol = pgm_read_byte(&sndPar[MODE_SND_VOLUME].grid->min);
		_mute = MUTE_ON;
		PORT(STMU_MUTE) &= ~STMU_MUTE_LINE;
	} else {
		vol = sndPar[MODE_SND_VOLUME].value;
		_mute = MUTE_OFF;
		PORT(STMU_MUTE) |= STMU_MUTE_LINE;
	}

	sndPar[MODE_SND_VOLUME].set(vol);

	switch (_aproc) {
	case AUDIOPROC_TDA7439:
		tda7439SetMute(_mute);
		break;
	case AUDIOPROC_TDA7312:
	case AUDIOPROC_TDA7313:
	case AUDIOPROC_TDA7314:
	case AUDIOPROC_TDA7315:
	case AUDIOPROC_TDA7318:
	case AUDIOPROC_PT2314:
		tda731xSetMute(_mute);
		break;
	case AUDIOPROC_TDA7448:
		tda7448SetMute(_mute);
	case AUDIOPROC_PGA2310:
		pga2310SetMute(_mute);
	default:
		break;
	}

	return;
}

uint8_t sndGetMute(void)
{
	return _mute;
}

void sndSetLoudness(uint8_t value)
{
	if (value == LOUDNESS_ON)
		_loudness = LOUDNESS_ON;
	else
		_loudness = LOUDNESS_OFF;

	if (_aproc == AUDIOPROC_TDA7313 || _aproc == AUDIOPROC_TDA7314 ||
		_aproc == AUDIOPROC_TDA7315 || _aproc == AUDIOPROC_PT2314)
		tda731xSetLoudness(_loudness);

	return;
}

uint8_t sndGetLoudness(void)
{
	return _loudness;
}

void sndNextParam(uint8_t *mode)
{
	do {					/* Skip unused params (with step = 0) */
		(*mode)++;
		if (*mode >= MODE_SND_GAIN0)
			*mode = MODE_SND_VOLUME;
	} while((pgm_read_byte(&sndPar[*mode].grid->step) == 0) &&
	        (*mode < MODE_SND_GAIN0) && (*mode != MODE_SND_VOLUME));

	return;
}

void sndChangeParam(uint8_t mode, int8_t diff)
{
	sndParam *param = sndParAddr(mode);
	param->value += diff;
	if (param->value > (int8_t)pgm_read_byte(&param->grid->max))
		param->value = (int8_t)pgm_read_byte(&param->grid->max);
	if (param->value < (int8_t)pgm_read_byte(&param->grid->min))
		param->value = (int8_t)pgm_read_byte(&param->grid->min);
	param->set(param->value);

	return;
}

void sndPowerOn(void)
{
	uint8_t i;

	sndSetMute(MUTE_ON);
	sndSetInput(_input);
	for (i = MODE_SND_VOLUME + 1; i < MODE_SND_GAIN0; i++)
		sndPar[i].set(sndPar[i].value);
	sndSetMute(MUTE_OFF);
	sndSetLoudness(_loudness);

	return;
}

void sndPowerOff(void)
{
	uint8_t i;

	for (i = 0; i < MODE_SND_END; i++)
		eeprom_update_byte((uint8_t*)EEPROM_VOLUME + i, sndPar[i].value);

	eeprom_update_byte((uint8_t*)EEPROM_INPUT, _input);
	eeprom_update_byte((uint8_t*)EEPROM_LOUDNESS, _loudness);
	eeprom_update_byte((uint8_t*)EEPROM_AUDIOPROC, _aproc);

	return;
}
