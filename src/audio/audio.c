#include "audio.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "../eeprom.h"
#include "../pins.h"

#ifdef _TDA7439
#include "tda7439.h"
#endif
#ifdef _TDA731X
#include "tda731x.h"
#endif
#ifdef _TDA7448
#include "tda7448.h"
#endif
#ifdef _PT232X
#include "pt232x.h"
#endif
#ifdef _TEA63X0
#include "tea63x0.h"
#endif
#ifdef _PGA2310
#include "pga2310.h"
#endif
#ifdef _TUNER_AUDIO
#include "../tuner/tuner.h"
#endif
#ifdef _R2S15904SP
#include "r2s15904sp.h"
#endif

static const sndGrid grid_0_0_0             PROGMEM = {  0,  0, 0.00 * 8};  // Not implemented
#if defined(_TDA7439) || defined(_TDA7448) || defined(_PT232X) || defined (_R2S15904SP)
static const sndGrid grid_n79_0_1           PROGMEM = {-79,  0, 1.00 * 8};  // -79..0dB with 1dB step
#endif
#if defined(_TDA7439) || defined(_TDA731X) || defined(_PT232X) || defined(_TEA63X0)
static const sndGrid grid_n14_14_2          PROGMEM = { -7,  7, 2.00 * 8};  // -14..14dB with 2dB step
#endif
#if defined(_TDA7439) || defined(_PGA2310) || defined (_R2S15904SP)
static const sndGrid grid_n15_15_1          PROGMEM = {-15, 15, 1.00 * 8};  // -15..15dB with 1dB step
#endif
#ifdef _TDA7439
static const sndGrid grid_n47_0_1           PROGMEM = {-47,  0, 1.00 * 8};  // -47..0dB with 1dB step
static const sndGrid grid_0_30_2            PROGMEM = {  0, 15, 2.00 * 8};  // 0..30dB with 2dB step
#endif
#ifdef _TDA731X
static const sndGrid grid_n78d75_0_1d25     PROGMEM = {-63,  0, 1.25 * 8};  // -78.75..0dB with 1.25dB step
static const sndGrid grid_n18d75_18d75_1d25 PROGMEM = {-15, 15, 1.25 * 8};  // -18.75..18.75dB with 1.25dB step
static const sndGrid grid_0_11d25_3d75      PROGMEM = {  0,  3, 3.75 * 8};  // 0..11.25dB with 3.75dB step
static const sndGrid grid_0_18d75_6d25      PROGMEM = {  0,  3, 6.25 * 8};  // 0..18.75dB with 6.25dB step
#endif
#if defined(_TDA7448) || defined(_PT232X)
static const sndGrid grid_n15_0_1           PROGMEM = {-15,  0, 1.00 * 8};  // -15..0dB with 1dB step
static const sndGrid grid_n7_7_1            PROGMEM = { -7,  7, 1.00 * 8};  // -7..7dB with 1dB step
#endif
#ifdef _PGA2310
static const sndGrid grid_n96_31_1          PROGMEM = {-96, 31, 1.00 * 8};  // -96..31dB with 1dB step
#endif
#ifdef _PT232X
static const sndGrid grid_0_6_6             PROGMEM = {  0,  1, 6.00 * 8};  // 0..6dB with 6dB step
#endif
#ifdef _TEA63X0
static const sndGrid grid_n66_20_2          PROGMEM = {-33, 10, 2.00 * 8};  // -66..20dB with 2dB step
static const sndGrid grid_n12_15_3          PROGMEM = { -4,  5, 3.00 * 8};  // -12..15dB with 3dB step
static const sndGrid grid_n12_12_3          PROGMEM = { -4,  4, 3.00 * 8};  // -12..12dB with 3dB step
#endif
#ifdef _TUNER_AUDIO
static const sndGrid grid_0_15_1            PROGMEM = {  0, 15, 1.00 * 8};  // 0..15dB with 1dB step
#endif
#if defined(_R2S15904SP)
static const sndGrid grid_n16_16_2          PROGMEM = { -8,  8, 2.00 * 8};  // -16..16dB with 2dB step
static const sndGrid grid_0_12_2            PROGMEM = {  0,  6, 2.00 * 8};  // 0..12dB with 2dB step
#endif

sndParam sndPar[MODE_SND_END];
Audioproc_type aproc;

static void setNothing() {}

#ifdef _TUNER_AUDIO
static void tunerAudioSetVolume()
{
    tunerSetVolume(sndPar[MODE_SND_VOLUME].value);
}
#endif

void sndInit()
{
    uint8_t i;

    // Load audio parameters stored in eeprom
    for (i = 0; i < MODE_SND_END; i++)
        sndPar[i].value = eeprom_read_byte((uint8_t *)EEPROM_VOLUME + i);
    eeprom_read_block(&aproc, (void *)EEPROM_AUDIOPROC, sizeof(Audioproc_type) - 1);

#if   !defined(_TDA7439) && !defined(_TDA731X) && !defined(_TDA7448) && !defined(_PT232X) && !defined(_TEA63X0) && !defined(_PGA2310) && !defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    aproc.ic = AUDIOPROC_NO;
#elif  defined(_TDA7439) && !defined(_TDA731X) && !defined(_TDA7448) && !defined(_PT232X) && !defined(_TEA63X0) && !defined(_PGA2310) && !defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    aproc.ic = AUDIOPROC_TDA7439;
#elif !defined(_TDA7439) &&  defined(_TDA731X) && !defined(_TDA7448) && !defined(_PT232X) && !defined(_TEA63X0) && !defined(_PGA2310) && !defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    if (aproc.ic < AUDIOPROC_TDA7312 || aproc.ic >= AUDIOPROC_PT2314)
        aproc.ic = AUDIOPROC_TDA7313;
#elif !defined(_TDA7439) && !defined(_TDA731X) &&  defined(_TDA7448) && !defined(_PT232X) && !defined(_TEA63X0) && !defined(_PGA2310) && !defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    aproc.ic = AUDIOPROC_TDA7448;
#elif !defined(_TDA7439) && !defined(_TDA731X) && !defined(_TDA7448) &&  defined(_PT232X) && !defined(_TEA63X0) && !defined(_PGA2310) && !defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    aproc.ic = AUDIOPROC_PT232X;
#elif !defined(_TDA7439) && !defined(_TDA731X) && !defined(_TDA7448) && !defined(_PT232X) &&  defined(_TEA63X0) && !defined(_PGA2310) && !defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    if (aproc.ic != AUDIOPROC_TEA6330)
        aproc.ic = AUDIOPROC_TEA6300;
#elif !defined(_TDA7439) && !defined(_TDA731X) && !defined(_TDA7448) && !defined(_PT232X) && !defined(_TEA63X0) &&  defined(_PGA2310) && !defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    aproc.ic = AUDIOPROC_PGA2310;
#elif !defined(_TDA7439) && !defined(_TDA731X) && !defined(_TDA7448) && !defined(_PT232X) && !defined(_TEA63X0) && !defined(_PGA2310) &&  defined(_TUNER_AUDIO) && !defined(_R2S15904SP)
    aproc.ic = AUDIOPROC_TUNER_IC;
#elif !defined(_TDA7439) && !defined(_TDA731X) && !defined(_TDA7448) && !defined(_PT232X) && !defined(_TEA63X0) && !defined(_PGA2310) && !defined(_TUNER_AUDIO) &&  defined(_R2S15904SP)
    aproc.ic = AUDIOPROC_R2S15904SP;
#else
    if (aproc.ic >= AUDIOPROC_END)
        aproc.ic = AUDIOPROC_NO;
#endif

#ifdef _PGA2310
    if (aproc.ic == AUDIOPROC_PGA2310)
        pga2310Init();
#endif

    // Init grid and functions with empty values
    for (i = 0; i < MODE_SND_END; i++) {
        sndPar[i].grid = &grid_0_0_0;
        sndPar[i].set = setNothing;
    }

    // Setup inputs
    static uint8_t inCnt;
    switch (aproc.ic) {
#ifdef _TDA7439
    case AUDIOPROC_TDA7439:
        inCnt = TDA7439_IN_CNT;
        break;
#endif
#ifdef _TDA731X
    case AUDIOPROC_TDA7312:
        inCnt = TDA7312_IN_CNT;
        break;
    case AUDIOPROC_TDA7313:
        inCnt = TDA7313_IN_CNT;
        break;
    case AUDIOPROC_TDA7314:
        inCnt = TDA7314_IN_CNT;
        break;
    case AUDIOPROC_TDA7315:
        inCnt = TDA7315_IN_CNT;
        break;
    case AUDIOPROC_TDA7318:
        inCnt = TDA7318_IN_CNT;
        break;
    case AUDIOPROC_PT2314:
        inCnt = PT2314_IN_CNT;
        break;
#endif
#ifdef _TDA7448
    case AUDIOPROC_TDA7448:
        inCnt = TDA7448_IN_CNT;
        break;
#endif
#ifdef _PT232X
    case AUDIOPROC_PT232X:
        inCnt = PT2323_IN_CNT;
        break;
#endif
#ifdef _TEA63X0
    case AUDIOPROC_TEA6300:
        inCnt = TEA6300_IN_CNT;
        break;
    case AUDIOPROC_TEA6330:
        inCnt = TEA6330_IN_CNT;
        break;
#endif
#ifdef _PGA2310
    case AUDIOPROC_PGA2310:
        inCnt = PGA2310_IN_CNT;
        break;
#endif
#ifdef _R2S15904SP
    case AUDIOPROC_R2S15904SP:
        inCnt = R2S15904SP_IN_CNT;
        break;
#endif
    default:
        inCnt = 1;
        break;
    }
    // Limit global input count
    if (aproc.inCnt > inCnt || aproc.inCnt == 0)
        aproc.inCnt = inCnt;
    // Limit current input
    if (aproc.input >= inCnt)
        aproc.input = 0;

    // Setup gain grid and functions
    const sndGrid *grid = &grid_0_0_0;
    void (*set)() = setNothing;
    switch (aproc.ic) {
#ifdef _TDA7439
    case AUDIOPROC_TDA7439:
        grid = &grid_0_30_2;
        set = tda7439SetInput;
        break;
#endif
#ifdef _TDA731X
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_PT2314:
        grid = &grid_0_11d25_3d75;
        set = tda731xSetInput;
        break;
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7318:
        grid = &grid_0_18d75_6d25;
        set = tda731xSetInput;
        break;
#endif
#ifdef _PT232X
    case AUDIOPROC_PT232X:
        grid = &grid_0_6_6;
        set = pt2323SetInput;
        break;
#endif
#ifdef _R2S15904SP
    case AUDIOPROC_R2S15904SP:
        grid = &grid_0_12_2;
        set = r2s15904spSetInput;
        break;
#endif
    default:
        break;
    }
    for (i = MODE_SND_GAIN0; i < MODE_SND_END; i++) {
        sndPar[i].grid = grid;
        sndPar[i].set = set;
    }

    // Setup audio parameters grid and functions
    switch (aproc.ic) {
#ifdef _TDA7439
    case AUDIOPROC_TDA7439:
        sndPar[MODE_SND_VOLUME].grid = &grid_n79_0_1;
        sndPar[MODE_SND_VOLUME].set = tda7439SetSpeakers;
        sndPar[MODE_SND_BASS].grid = &grid_n14_14_2;
        sndPar[MODE_SND_BASS].set = tda7439SetBMT;
        sndPar[MODE_SND_MIDDLE].grid = &grid_n14_14_2;
        sndPar[MODE_SND_MIDDLE].set = tda7439SetBMT;
        sndPar[MODE_SND_TREBLE].grid = &grid_n14_14_2;
        sndPar[MODE_SND_TREBLE].set = tda7439SetBMT;
        sndPar[MODE_SND_PREAMP].grid = &grid_n47_0_1;
        sndPar[MODE_SND_PREAMP].set = tda7439SetPreamp;
        sndPar[MODE_SND_BALANCE].grid = &grid_n15_15_1;
        sndPar[MODE_SND_BALANCE].set = tda7439SetSpeakers;
        break;
#endif
#ifdef _TDA731X
    case AUDIOPROC_TDA7312:
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7315:
    case AUDIOPROC_TDA7318:
    case AUDIOPROC_PT2314:
        sndPar[MODE_SND_VOLUME].grid = &grid_n78d75_0_1d25;
        sndPar[MODE_SND_VOLUME].set = tda731xSetVolume;
        sndPar[MODE_SND_BASS].grid = &grid_n14_14_2;
        sndPar[MODE_SND_BASS].set = tda731xSetBT;
        sndPar[MODE_SND_TREBLE].grid = &grid_n14_14_2;
        sndPar[MODE_SND_TREBLE].set = tda731xSetBT;
        sndPar[MODE_SND_BALANCE].grid = &grid_n18d75_18d75_1d25;
        sndPar[MODE_SND_BALANCE].set = tda731xSetSpeakers;
        switch (aproc.ic) {
        case AUDIOPROC_TDA7313:
        case AUDIOPROC_TDA7314:
        case AUDIOPROC_TDA7318:
            sndPar[MODE_SND_FRONTREAR].grid = &grid_n18d75_18d75_1d25;
            sndPar[MODE_SND_FRONTREAR].set = tda731xSetSpeakers;
            break;
        default:
            break;
        }
        break;
#endif
#ifdef _TDA7448
    case AUDIOPROC_TDA7448:
        sndPar[MODE_SND_VOLUME].grid = &grid_n79_0_1;
        sndPar[MODE_SND_VOLUME].set = tda7448SetSpeakers;
        sndPar[MODE_SND_FRONTREAR].grid = &grid_n7_7_1;
        sndPar[MODE_SND_FRONTREAR].set = tda7448SetSpeakers;
        sndPar[MODE_SND_BALANCE].grid = &grid_n7_7_1;
        sndPar[MODE_SND_BALANCE].set = tda7448SetSpeakers;
        sndPar[MODE_SND_CENTER].grid = &grid_n15_0_1;
        sndPar[MODE_SND_CENTER].set = tda7448SetSpeakers;
        sndPar[MODE_SND_SUBWOOFER].grid = &grid_n15_0_1;
        sndPar[MODE_SND_SUBWOOFER].set = tda7448SetSpeakers;
        break;
#endif
#ifdef _PT232X
    case AUDIOPROC_PT232X:
        sndPar[MODE_SND_VOLUME].grid = &grid_n79_0_1;
        sndPar[MODE_SND_VOLUME].set = pt2322SetVolume;
        sndPar[MODE_SND_BASS].grid = &grid_n14_14_2;
        sndPar[MODE_SND_BASS].set = pt2322SetBMT;
        sndPar[MODE_SND_MIDDLE].grid = &grid_n14_14_2;
        sndPar[MODE_SND_MIDDLE].set = pt2322SetBMT;
        sndPar[MODE_SND_TREBLE].grid = &grid_n14_14_2;
        sndPar[MODE_SND_TREBLE].set = pt2322SetBMT;
        sndPar[MODE_SND_FRONTREAR].grid = &grid_n7_7_1;
        sndPar[MODE_SND_FRONTREAR].set = pt2322SetSpeakers;
        sndPar[MODE_SND_BALANCE].grid = &grid_n7_7_1;
        sndPar[MODE_SND_BALANCE].set = pt2322SetSpeakers;
        sndPar[MODE_SND_CENTER].grid = &grid_n15_0_1;
        sndPar[MODE_SND_CENTER].set = pt2322SetSpeakers;
        sndPar[MODE_SND_SUBWOOFER].grid = &grid_n15_0_1;
        sndPar[MODE_SND_SUBWOOFER].set = pt2322SetSpeakers;
        break;
#endif
#ifdef _TEA63X0
    case AUDIOPROC_TEA6300:
    case AUDIOPROC_TEA6330:
        sndPar[MODE_SND_VOLUME].grid = &grid_n66_20_2;
        sndPar[MODE_SND_VOLUME].set = tea63x0SetVolume;
        sndPar[MODE_SND_BASS].grid = &grid_n12_15_3;
        sndPar[MODE_SND_BASS].set = tea63x0SetBT;
        sndPar[MODE_SND_TREBLE].grid = &grid_n12_12_3;
        sndPar[MODE_SND_TREBLE].set = tea63x0SetBT;
        sndPar[MODE_SND_FRONTREAR].grid = &grid_n14_14_2;
        sndPar[MODE_SND_FRONTREAR].set = tea63x0SetSpeakers;
        sndPar[MODE_SND_BALANCE].grid = &grid_n14_14_2;
        sndPar[MODE_SND_BALANCE].set = tea63x0SetVolume;
        break;
#endif
#ifdef _PGA2310
    case AUDIOPROC_PGA2310:
        sndPar[MODE_SND_VOLUME].grid = &grid_n96_31_1;
        sndPar[MODE_SND_VOLUME].set = pga2310SetSpeakers;
        sndPar[MODE_SND_BALANCE].grid = &grid_n15_15_1;
        sndPar[MODE_SND_BALANCE].set = pga2310SetSpeakers;
        break;
#endif
#ifdef _TUNER_AUDIO
    case AUDIOPROC_TUNER_IC:
        sndPar[MODE_SND_VOLUME].grid = &grid_0_15_1;
        sndPar[MODE_SND_VOLUME].set = tunerAudioSetVolume;
        break;
#endif
#ifdef _R2S15904SP
    case AUDIOPROC_R2S15904SP:
        sndPar[MODE_SND_VOLUME].grid = &grid_n79_0_1;
        sndPar[MODE_SND_VOLUME].set = r2s15904spSetSpeakers;
        sndPar[MODE_SND_BALANCE].grid = &grid_n15_15_1;
        sndPar[MODE_SND_BALANCE].set = r2s15904spSetSpeakers;
        sndPar[MODE_SND_BASS].grid = &grid_n16_16_2;
        sndPar[MODE_SND_BASS].set = r2s15904spSetBT;
        sndPar[MODE_SND_TREBLE].grid = &grid_n16_16_2;
        sndPar[MODE_SND_TREBLE].set = r2s15904spSetBT;
        break;
#endif
    default:
        break;
    }
}

void sndSetInput(uint8_t input)
{
    if (input >= aproc.inCnt)
        input = 0;
    aproc.input = input;

    switch (aproc.ic) {
#ifdef _TDA7439
    case AUDIOPROC_TDA7439:
        tda7439SetInput();
        break;
#endif
#ifdef _TDA731X
    case AUDIOPROC_TDA7312:
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7315:
    case AUDIOPROC_TDA7318:
    case AUDIOPROC_PT2314:
        tda731xSetInput();
        break;
#endif
#ifdef _PT232X
    case AUDIOPROC_PT232X:
        pt2323SetInput();
        break;
#endif
#ifdef _TEA63X0
    case AUDIOPROC_TEA6300:
        tea63x0SetInputMute();
        break;
#endif
#ifdef _R2S15904SP
    case AUDIOPROC_R2S15904SP:
        r2s15904spSetInput();
        break;
#endif
    default:
        break;
    }
}

void sndSetMute(uint8_t value)
{
    aproc.mute = value;

#ifndef _NO_MUTE_PORT
    if (aproc.mute)
        CLR(STMU_MUTE);
    else
        SET(STMU_MUTE);
#endif

    switch (aproc.ic) {
#ifdef _TDA7439
    case AUDIOPROC_TDA7439:
        tda7439SetMute();
        break;
#endif
#ifdef _TDA731X
    case AUDIOPROC_TDA7312:
    case AUDIOPROC_TDA7313:
    case AUDIOPROC_TDA7314:
    case AUDIOPROC_TDA7315:
    case AUDIOPROC_TDA7318:
    case AUDIOPROC_PT2314:
        tda731xSetMute();
        break;
#endif
#ifdef _TDA7448
    case AUDIOPROC_TDA7448:
        tda7448SetMute();
        break;
#endif
#ifdef _PT232X
    case AUDIOPROC_PT232X:
        pt232xSetSndFunc();
        break;
#endif
#ifdef _TEA63X0
    case AUDIOPROC_TEA6300:
    case AUDIOPROC_TEA6330:
        tea63x0SetInputMute();
        break;
#endif
#ifdef _PGA2310
    case AUDIOPROC_PGA2310:
        pga2310SetMute();
        break;
#endif
#ifdef _TUNER_AUDIO
    case AUDIOPROC_TUNER_IC:
        tunerSetMute(value);
        break;
#endif
#ifdef _R2S15904SP
    case AUDIOPROC_R2S15904SP:
        r2s15904spSetMute();
        break;
#endif
    default:
        break;
    }
}

void sndSetExtra()
{
#ifdef _TDA731X
    if (aproc.ic == AUDIOPROC_TDA7313 || aproc.ic == AUDIOPROC_TDA7314 ||
            aproc.ic == AUDIOPROC_TDA7315 || aproc.ic == AUDIOPROC_PT2314)
        tda731xSetInput();
#endif
#ifdef _TUNER_AUDIO
    if (aproc.ic == AUDIOPROC_TUNER_IC)
        tunerSetBass(aproc.extra & APROC_EXTRA_LOUDNESS);
#endif
#ifdef _PT232X
    if (aproc.ic == AUDIOPROC_PT232X)
        pt232xSetSndFunc();
#endif
#ifdef _R2S15904SP
    if (aproc.ic == AUDIOPROC_R2S15904SP)
        r2s15904spSetInput();
#endif
}

void sndSwitchExtra(uint8_t extra)
{
    aproc.extra ^= extra;

    sndSetExtra();
}

void sndNextParam(uint8_t *mode)
{
    do { // Skip unused params (with step = 0)
        (*mode)++;
        if (*mode >= MODE_SND_GAIN0)
            *mode = MODE_SND_VOLUME;
    } while ((pgm_read_byte(&sndPar[*mode].grid->step) == 0) &&
             (*mode < MODE_SND_GAIN0) && (*mode != MODE_SND_VOLUME));
}

void sndChangeParam(uint8_t mode, int8_t diff)
{
    sndParam *param = &sndPar[mode];
    param->value += diff;
    if (param->value > (int8_t)pgm_read_byte(&param->grid->max))
        param->value = (int8_t)pgm_read_byte(&param->grid->max);
    if (param->value < (int8_t)pgm_read_byte(&param->grid->min))
        param->value = (int8_t)pgm_read_byte(&param->grid->min);
    param->set();
}

void sndPowerOn()
{
    int8_t i;

#ifdef _PT232X
    if (aproc.ic == AUDIOPROC_PT232X)
        pt232xReset();
#endif

    sndSetMute(1);
    sndSetInput(aproc.input);

    sndSetExtra();

    for (i = MODE_SND_GAIN0 - 1; i >= MODE_SND_VOLUME; i--)
        sndPar[i].set();

    sndSetMute(0);
}

void sndPowerOff()
{
    uint8_t i;

    for (i = 0; i < MODE_SND_END; i++)
        eeprom_update_byte((uint8_t *)EEPROM_VOLUME + i, sndPar[i].value);

    eeprom_update_byte((uint8_t *)EEPROM_APROC_EXTRA, aproc.extra);
    eeprom_update_byte((uint8_t *)EEPROM_INPUT, aproc.input);
}
