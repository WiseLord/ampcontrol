#include "r2s15904sp.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include "audio.h"
#include "../pins.h"

static void r2s15904spDelay()
{
    _delay_us(4);
}

static void r2s15904spSendData(uint16_t data)
{
    uint8_t i;

    for (i = 0; i < 16; i++) {
        // Set data
        if (data & 0x8000) {
            SET(SPISW_DI);
        } else {
            CLR(SPISW_DI);
        }
        r2s15904spDelay();
        // Set clock
        SET(SPISW_CLK);
        r2s15904spDelay();
        // Clear data but last bit
        if (i == 15) {
            SET(SPISW_DI);
        } else {
            CLR(SPISW_DI);
        }
        r2s15904spDelay();
        // Clear clock
        CLR(SPISW_CLK);
        r2s15904spDelay();

        data <<= 1;
    }
    // Latch data
    CLR(SPISW_DI);
    r2s15904spDelay();
    // Set clock
    SET(SPISW_CLK);
    r2s15904spDelay();
    CLR(SPISW_CLK);
    r2s15904spDelay();
}

void r2s15904spSetSpeakers()
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

    r2s15904spSendData(((-spRight) << 9) | ((-spLeft) << 2) | R2S15904SP_VOLCTRL);
}

void r2s15904spSetBT()
{
    // Limited by -16..+16dB, but can accept -30..+30dB range
    int8_t bass = sndPar[MODE_SND_BASS].value;
    int8_t treble = sndPar[MODE_SND_TREBLE].value;

    // Recalculate to raw with handling negative values
    bass = (bass <= 0) ? (-bass) : (bass | 0x10);
    treble = (treble <= 0) ? (-treble) : (treble | 0x10);

    r2s15904spSendData((bass << 11) | (treble << 6) | R2S15904SP_BTCTRL);
}

void r2s15904spSetInput()
{
    int8_t val = sndPar[MODE_SND_GAIN0 + aproc.input].value;
    uint8_t in = R2S15904SP_IN_CNT - 1 - aproc.input;
    uint16_t toneBypass = (aproc.extra & APROC_EXTRA_TONE_BYPASS) ? 0 : R2S15904SP_TONE_EN;

    r2s15904spSendData((in << 13) | (val << 10) | toneBypass);
}

void r2s15904spSetMute()
{
    if (aproc.mute) {
        r2s15904spSendData(R2S15904SP_MUTE | R2S15904SP_VOLCTRL);
    } else {
        r2s15904spSetSpeakers();
    }
}
