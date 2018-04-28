#include "tda7448.h"
#include "audio.h"

#include <avr/pgmspace.h>
#include "../i2c.h"

void tda7448SetSpeakers()
{
    uint8_t i;
    uint8_t i2cData;
    int8_t sp[TDA7448_END];
    int8_t volMin = pgm_read_byte(&sndPar[MODE_SND_VOLUME].grid->min);

    sp[TDA7448_FRONT_LEFT] = sndPar[MODE_SND_VOLUME].value;
    sp[TDA7448_FRONT_RIGHT] = sndPar[MODE_SND_VOLUME].value;
    sp[TDA7448_REAR_LEFT] = sndPar[MODE_SND_VOLUME].value;
    sp[TDA7448_REAR_RIGHT] = sndPar[MODE_SND_VOLUME].value;
    sp[TDA7448_CENTER] = sndPar[MODE_SND_VOLUME].value;
    sp[TDA7448_SUBWOOFER] = sndPar[MODE_SND_VOLUME].value;

    if (sndPar[MODE_SND_BALANCE].value > 0) {
        sp[TDA7448_FRONT_LEFT] -= sndPar[MODE_SND_BALANCE].value;
        sp[TDA7448_REAR_LEFT] -= sndPar[MODE_SND_BALANCE].value;
    } else {
        sp[TDA7448_FRONT_RIGHT] += sndPar[MODE_SND_BALANCE].value;
        sp[TDA7448_REAR_RIGHT] += sndPar[MODE_SND_BALANCE].value;
    }

    if (sndPar[MODE_SND_FRONTREAR].value > 0) {
        sp[TDA7448_REAR_LEFT] -= sndPar[MODE_SND_FRONTREAR].value;
        sp[TDA7448_REAR_RIGHT] -= sndPar[MODE_SND_FRONTREAR].value;
    } else {
        sp[TDA7448_FRONT_LEFT] += sndPar[MODE_SND_FRONTREAR].value;
        sp[TDA7448_FRONT_RIGHT] += sndPar[MODE_SND_FRONTREAR].value;
    }

    sp[TDA7448_CENTER] += sndPar[MODE_SND_CENTER].value;
    sp[TDA7448_SUBWOOFER] += sndPar[MODE_SND_SUBWOOFER].value;

    I2CStart(TDA7448_I2C_ADDR);
    I2CWriteByte(TDA7448_AUTO_INC);
    for (i = 0; i < TDA7448_END; i++) {
        // Limit values sent to bus
        if (sp[i] < volMin)
            sp[i] = volMin;
        i2cData = -sp[i];
        // Jump at -72db in raw data according the datasheet
        if (i2cData >= 72)
            i2cData += 56;
        I2CWriteByte(i2cData);
    }
    I2CStop();
}

void tda7448SetMute()
{
    uint8_t i;

    if (aproc.mute) {
        I2CStart(TDA7448_I2C_ADDR);
        I2CWriteByte(TDA7448_AUTO_INC);
        for (i = 0; i < TDA7448_END; i++)
            I2CWriteByte(TDA7448_MUTE);
        I2CStop();
    } else {
        tda7448SetSpeakers();
    }
}
