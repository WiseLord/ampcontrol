#include "rds.h"
#include "../display.h"

static uint8_t rdsText[] = "        ";
static uint8_t rdsFlag = 0;

uint8_t *rdsGetText(void)
{
	return rdsText;
}

void rdsSetBlocks(uint8_t *rdsBlock)
{
	/*
	 * rdsBlock[0..1] - RDS block A
	 * rdsBlock[2..3] - RDS block B
	 * rdsBlock[4..5] - RDS block C
	 * rdsBlock[6..7] - RDS block D
	 */

	if (rdsBlock[6] >= 0x20 && rdsBlock[6] < 0xFF)
		rdsText[(rdsBlock[3] & 0x03) * 2] = rdsBlock[6];
	if (rdsBlock[7] >= 0x20 && rdsBlock[7] < 0xFF)
		rdsText[(rdsBlock[3] & 0x03) * 2 + 1] = rdsBlock[7];

	rdsFlag = 1;

	return;
}

void rdsDisable(void)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
		rdsText[i] = ' ';

	rdsFlag = 0;

	return;
}

uint8_t rdsGetFlag(void)
{
	return rdsFlag;
}
