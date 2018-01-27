#ifndef SPISW_H
#define SPISW_H

#include <inttypes.h>

// Data transfer order
#define SPISW_DORD_MSB_FIRST    0
#define SPISW_DORD_LSB_FIRST    1

void SPIswInit(uint8_t dataOrder);
void SPIswSendByte(uint8_t data);

#endif
