#include "rds.h"

static char rdsText[9];
static uint8_t rdsFlag = 0;

char *rdsGetText()
{
    return rdsText;
}

void rdsSetBlocks(uint8_t *rdsBlock)
{
    // rdsBlock[0..1] - RDS block A
    // rdsBlock[2..3] - RDS block B
    // rdsBlock[4..5] - RDS block C
    // rdsBlock[6..7] - RDS block D

    uint8_t i;
    char rdsChar;

    uint8_t rdsVersion = (rdsBlock[2] & 0x08) >> 3;
    uint8_t rdsGroup   = (rdsBlock[2] & 0xF0) >> 4;
    uint8_t rdsIndex   = (rdsBlock[3] & 0x03) >> 0;

    if (rdsVersion == (0x00 & 0x08)) {                      // RDS version = 0
        if (rdsGroup == (0x00 & 0xF0)) {                    // RDS group = 0 (RDS0)
            for (i = 0; i < 2; i++) {
                rdsChar = rdsBlock[6 + i];
                if (rdsChar >= 0x20 && rdsChar < 0x80)
                    rdsText[rdsIndex * 2 + i] = rdsChar;
            }
            rdsFlag = RDS_FLAG_INIT;
        }
    }
}

void rdsDisable()
{
    uint8_t i;

    for (i = 0; i < 8; i++)
        rdsText[i] = ' ';

    rdsFlag = 0;
}

uint8_t rdsGetFlag()
{
    if (rdsFlag)
        rdsFlag--;

    return rdsFlag;
}
