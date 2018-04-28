#include "pt232x.h"
#include "audio.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

void pt232xReset()
{
    I2CStart(PT2322_I2C_ADDR);
    I2CWriteByte(PT2322_CREAR_REGS);
    I2CStop();

    I2CStart(PT2322_I2C_ADDR);
    I2CWriteByte(PT2322_INPUT_SW);
    I2CStop();

    I2CStart(PT2323_I2C_ADDR);
    I2CWriteByte(PT2323_UNMUTE_ALL);
    I2CStop();
}

void pt2322SetVolume()
{
    int8_t val = -sndPar[MODE_SND_VOLUME].value;

    I2CStart(PT2322_I2C_ADDR);
    I2CWriteByte(PT2322_VOL_HI | (val / 10));
    I2CWriteByte(PT2322_VOL_LO | (val % 10));
    I2CStop();
}

void pt2322SetBMT()
{
    int8_t val;
    uint8_t mode = MODE_SND_BASS;
    uint8_t param = PT2322_BASS;

    I2CStart(PT2322_I2C_ADDR);
    while (mode <= MODE_SND_TREBLE) {
        val = sndPar[mode++].value;
        I2CWriteByte(param | (val > 0 ? 15 - val : 7 + val));
        param += 0b00010000;
    }
    I2CStop();
}

void pt2322SetSpeakers()
{
    int8_t spFrontLeft = 0;
    int8_t spFrontRight = 0;
    int8_t spRearLeft = 0;
    int8_t spRearRight = 0;

    if (sndPar[MODE_SND_BALANCE].value > 0) {
        spFrontLeft += sndPar[MODE_SND_BALANCE].value;
        spRearLeft += sndPar[MODE_SND_BALANCE].value;
    } else {
        spFrontRight -= sndPar[MODE_SND_BALANCE].value;
        spRearRight -= sndPar[MODE_SND_BALANCE].value;
    }

    if (sndPar[MODE_SND_FRONTREAR].value > 0) {
        spRearLeft += sndPar[MODE_SND_FRONTREAR].value;
        spRearRight += sndPar[MODE_SND_FRONTREAR].value;
    } else {
        spFrontLeft -= sndPar[MODE_SND_FRONTREAR].value;
        spFrontRight -= sndPar[MODE_SND_FRONTREAR].value;
    }

    I2CStart(PT2322_I2C_ADDR);
    I2CWriteByte(PT2322_TRIM_FL | spFrontLeft);
    I2CWriteByte(PT2322_TRIM_FR | spFrontRight);
    I2CWriteByte(PT2322_TRIM_CT | -sndPar[MODE_SND_CENTER].value);
    I2CWriteByte(PT2322_TRIM_RL | spRearLeft);
    I2CWriteByte(PT2322_TRIM_RR | spRearRight);
    I2CWriteByte(PT2322_TRIM_SB | -sndPar[MODE_SND_SUBWOOFER].value);
    I2CStop();
}

void pt2323SetInput()
{
    I2CStart(PT2323_I2C_ADDR);
    I2CWriteByte(PT2323_INPUT_SWITCH | (PT2323_INPUT_ST1 - aproc.input));
    I2CWriteByte(PT2323_MIX | sndPar[MODE_SND_GAIN0 + aproc.input].value);
    I2CStop();
}

void pt232xSetSndFunc()
{
    uint8_t sndFunc = PT2322_FUNCTION;

    if (aproc.mute)
        sndFunc |= PT2322_MUTE_ON;
    if (!(aproc.extra & APROC_EXTRA_EFFECT3D))
        sndFunc |= PT2322_3D_OFF;
    if (aproc.extra & APROC_EXTRA_TONE_BYPASS)
        sndFunc |= PT2322_TONE_OFF;

    I2CStart(PT2322_I2C_ADDR);
    I2CWriteByte(sndFunc);
    I2CStop();

    I2CStart(PT2323_I2C_ADDR);
    I2CWriteByte(PT2323_ENH_SURR | !(aproc.extra & APROC_EXTRA_SURROUND));
    I2CStop();
}
