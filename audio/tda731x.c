#include "tda731x.h"
#include "audio.h"

#include "../i2c.h"

void tda731xSetVolume()
{
    I2CStart(TDA731X_I2C_ADDR);
    I2CWriteByte(TDA731X_VOLUME | -sndPar[MODE_SND_VOLUME].value);
    I2CStop();
}

void tda731xSetBT()
{
    int8_t val;

    I2CStart(TDA731X_I2C_ADDR);
    val = sndPar[MODE_SND_BASS].value;
    I2CWriteByte(TDA731X_BASS | (val > 0 ? 15 - val : 7 + val));
    val = sndPar[MODE_SND_TREBLE].value;
    I2CWriteByte(TDA731X_TREBLE | (val > 0 ? 15 - val : 7 + val));
    I2CStop();
}

void tda731xSetSpeakers()
{
    int8_t spFrontLeft = 0;
    int8_t spFrontRight = 0;

    int8_t spRearLeft = 0;
    int8_t spRearRight = 0;

    if (sndPar[MODE_SND_BALANCE].value > 0) {
        spFrontLeft -= sndPar[MODE_SND_BALANCE].value;
        spRearLeft -= sndPar[MODE_SND_BALANCE].value;
    } else {
        spFrontRight += sndPar[MODE_SND_BALANCE].value;
        spRearRight += sndPar[MODE_SND_BALANCE].value;
    }
    if (sndPar[MODE_SND_FRONTREAR].value > 0) {
        spRearLeft -= sndPar[MODE_SND_FRONTREAR].value;
        spRearRight -= sndPar[MODE_SND_FRONTREAR].value;
    } else {
        spFrontLeft += sndPar[MODE_SND_FRONTREAR].value;
        spFrontRight += sndPar[MODE_SND_FRONTREAR].value;
    }

    I2CStart(TDA731X_I2C_ADDR);
    I2CWriteByte(TDA731X_SP_REAR_LEFT | -spRearLeft);
    I2CWriteByte(TDA731X_SP_REAR_RIGHT | -spRearRight);
    I2CWriteByte(TDA731X_SP_FRONT_LEFT | -spFrontLeft);
    I2CWriteByte(TDA731X_SP_FRONT_RIGHT | -spFrontRight);
    I2CStop();
}

void tda731xSetInput()
{
    I2CStart(TDA731X_I2C_ADDR);
    I2CWriteByte(TDA731X_SW | (3 - sndPar[MODE_SND_GAIN0 + aproc.input].value) << 3 | !
                 (aproc.extra & APROC_EXTRA_LOUDNESS) << 2 | aproc.input);
    I2CStop();
}

void tda731xSetMute()
{
    I2CStart(TDA731X_I2C_ADDR);
    if (aproc.mute) {
        I2CWriteByte(TDA731X_SP_REAR_LEFT | TDA731X_MUTE);
        I2CWriteByte(TDA731X_SP_REAR_RIGHT | TDA731X_MUTE);
        I2CWriteByte(TDA731X_SP_FRONT_LEFT | TDA731X_MUTE);
        I2CWriteByte(TDA731X_SP_FRONT_RIGHT | TDA731X_MUTE);
    } else {
        tda731xSetSpeakers();
    }
    I2CStop();
}
