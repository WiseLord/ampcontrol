#include "tda7439.h"

#include <avr/eeprom.h>

#include "../i2c.h"
#include "../eeprom.h"
#include "../pins.h"

static uint8_t chan;
static uint8_t mute;

static sndParam sndPar[SND_PARAM_COUNT] = {
    {0x00, 0xB1, 0x00, 0x08},   // Volume
    {0x00, 0xF9, 0x07, 0x10},   // Bass
    {0x00, 0xF9, 0x07, 0x10},   // Middle
    {0x00, 0xF9, 0x07, 0x10},   // Treble
    {0x00, 0xD1, 0x00, 0x08},   // Preamp
    {0x00, 0xEB, 0x15, 0x08},   // Balance
    {0x00, 0x00, 0x0F, 0x10},   // Gain 0
    {0x00, 0x00, 0x0F, 0x10},   // Gain 1
    {0x00, 0x00, 0x0F, 0x10},   // Gain 2
    {0x00, 0x00, 0x0F, 0x10},   // Gain 3
};


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
    setVolume(sndPar[SND_VOLUME].value);

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

    sndPar[SND_VOLUME].value  = eeprom_read_byte((uint8_t *)MODE_SND_VOLUME);
    sndPar[SND_BASS].value    = eeprom_read_byte((uint8_t *)MODE_SND_BASS);
    sndPar[SND_MIDDLE].value  = eeprom_read_byte((uint8_t *)MODE_SND_MIDDLE);
    sndPar[SND_TREBLE].value  = eeprom_read_byte((uint8_t *)MODE_SND_TREBLE);
    sndPar[SND_PREAMP].value  = eeprom_read_byte((uint8_t *)MODE_SND_PREAMP);
    sndPar[SND_BALANCE].value = eeprom_read_byte((uint8_t *)MODE_SND_BALANCE);
    sndPar[SND_GAIN0].value   = eeprom_read_byte((uint8_t *)MODE_SND_GAIN0);
    sndPar[SND_GAIN1].value   = eeprom_read_byte((uint8_t *)MODE_SND_GAIN1);
    sndPar[SND_GAIN2].value   = eeprom_read_byte((uint8_t *)MODE_SND_GAIN2);
    sndPar[SND_GAIN3].value   = eeprom_read_byte((uint8_t *)MODE_SND_GAIN3);

    sndPar[SND_VOLUME].label  = txtLabels[MODE_SND_VOLUME];
    sndPar[SND_BASS].label    = txtLabels[MODE_SND_BASS];
    sndPar[SND_MIDDLE].label  = txtLabels[MODE_SND_MIDDLE];
    sndPar[SND_TREBLE].label  = txtLabels[MODE_SND_TREBLE];
    sndPar[SND_PREAMP].label  = txtLabels[MODE_SND_PREAMP];
    sndPar[SND_BALANCE].label = txtLabels[MODE_SND_BALANCE];
    sndPar[SND_GAIN0].label   = txtLabels[MODE_SND_GAIN0];
    sndPar[SND_GAIN1].label   = txtLabels[MODE_SND_GAIN1];
    sndPar[SND_GAIN2].label   = txtLabels[MODE_SND_GAIN2];
    sndPar[SND_GAIN3].label   = txtLabels[MODE_SND_GAIN3];

    chan = eeprom_read_byte((uint8_t *)EEPROM_INPUT);

    sndPar[SND_VOLUME].set = setVolume;
    sndPar[SND_BASS].set = setBass;
    sndPar[SND_MIDDLE].set = setMiddle;
    sndPar[SND_TREBLE].set = setTreble;
    sndPar[SND_PREAMP].set = setPreamp;
    sndPar[SND_BALANCE].set = setBalance;

    for (i = 0; i < CHAN_CNT; i++)
        sndPar[SND_GAIN0 + i].set = setGain;

    return;
}

void setAudioParams(void)
{
    muteVolume();
    setChan(chan);
    setBass(sndPar[SND_BASS].value);
    setPreamp(sndPar[SND_PREAMP].value);
    setMiddle(sndPar[SND_MIDDLE].value);
    setTreble(sndPar[SND_TREBLE].value);
    unmuteVolume();

    return;
}

void saveAudioParams(void)
{
    eeprom_update_byte((uint8_t *)MODE_SND_VOLUME,  sndPar[SND_VOLUME].value);
    eeprom_update_byte((uint8_t *)MODE_SND_BASS,    sndPar[SND_BASS].value );
    eeprom_update_byte((uint8_t *)MODE_SND_MIDDLE,  sndPar[SND_MIDDLE].value);
    eeprom_update_byte((uint8_t *)MODE_SND_TREBLE,  sndPar[SND_TREBLE].value);
    eeprom_update_byte((uint8_t *)MODE_SND_PREAMP,  sndPar[SND_PREAMP].value);
    eeprom_update_byte((uint8_t *)MODE_SND_BALANCE, sndPar[SND_BALANCE].value);
    eeprom_update_byte((uint8_t *)MODE_SND_GAIN0,   sndPar[SND_GAIN0].value);
    eeprom_update_byte((uint8_t *)MODE_SND_GAIN1,   sndPar[SND_GAIN1].value);
    eeprom_update_byte((uint8_t *)MODE_SND_GAIN2,   sndPar[SND_GAIN2].value);
    eeprom_update_byte((uint8_t *)MODE_SND_GAIN3,   sndPar[SND_GAIN3].value);

    eeprom_update_byte((uint8_t *)EEPROM_INPUT, chan);

    return;
}
