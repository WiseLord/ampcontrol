#include "pga2310.h"

#include <avr/pgmspace.h>
#include "audio.h"
#include "../spisw.h"
#include "../pins.h"

static void pga2310SendGainLevels(uint8_t right, uint8_t left)
{
    CLR(SPISW_CE);
    SPIswSendByte(right << 1);
    SPIswSendByte(left << 1);
    SET(SPISW_CE);
}

void pga2310Init()
{
    SPIswInit(SPISW_DORD_MSB_FIRST);

    SET(SPISW_CE);
}

void pga2310SetSpeakers()
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

    pga2310SendGainLevels(96 + spRight, 96 + spLeft);
}

void pga2310SetMute()
{
    if (aproc.mute) {
        pga2310SendGainLevels(PGA2310_MUTE, PGA2310_MUTE);
    } else {
        pga2310SetSpeakers();
    }
}
