#include "tda7439.h"
#include "audio.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

void tda7439SetSpeakers()
{
    int8_t spLeft = sndPar[MODE_SND_VOLUME].value;
    int8_t spRight = sndPar[MODE_SND_VOLUME].value;
    int8_t volMin = pgm_read_byte(&sndPar[MODE_SND_VOLUME].grid->min);

    if (sndPar[MODE_SND_BALANCE].value > 0) {
        spLeft -= sndPar[MODE_SND_BALANCE].value;
        if (spLeft < volMin)
            spLeft = volMin;
    } else {
        spRight += sndPar[MODE_SND_BALANCE].value;
        if (spRight < volMin)
            spRight = volMin;
    }

    I2CStart(TDA7439_I2C_ADDR);
    I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
    I2CWriteByte(-spRight);
    I2CWriteByte(-spLeft);
    I2CStop();
}

void tda7439SetBMT()
{
    int8_t val;
    uint8_t mode;

    I2CStart(TDA7439_I2C_ADDR);
    I2CWriteByte(TDA7439_BASS | TDA7439_AUTO_INC);
    for (mode = MODE_SND_BASS; mode <= MODE_SND_TREBLE; mode++) {
        val = sndPar[mode].value;
        I2CWriteByte(val > 0 ? 15 - val : 7 + val);
    }
    I2CStop();
}

void tda7439SetPreamp()
{
    I2CStart(TDA7439_I2C_ADDR);
    I2CWriteByte(TDA7439_PREAMP);
    I2CWriteByte(-sndPar[MODE_SND_PREAMP].value);
    I2CStop();
}

void tda7439SetInput()
{
    I2CStart(TDA7439_I2C_ADDR);
    I2CWriteByte(TDA7439_INPUT_SELECT | TDA7439_AUTO_INC);
    I2CWriteByte(TDA7439_IN_CNT - 1 - aproc.input);
    I2CWriteByte(sndPar[MODE_SND_GAIN0 + aproc.input].value);
    I2CStop();
}

void tda7439SetMute()
{
    if (aproc.mute) {
        I2CStart(TDA7439_I2C_ADDR);
        I2CWriteByte(TDA7439_VOLUME_RIGHT | TDA7439_AUTO_INC);
        I2CWriteByte(TDA7439_SPEAKER_MUTE);
        I2CWriteByte(TDA7439_SPEAKER_MUTE);
        I2CStop();
    } else {
        tda7439SetSpeakers();
    }
}
