#ifndef I2C_H
#define I2C_H

#include <inttypes.h>

#define I2C_NOACK   0
#define I2C_ACK     1
#define I2C_READ    1

void I2CInit();

uint8_t I2CStart(uint8_t addr);
void I2CStop();

uint8_t I2CWriteByte(uint8_t data);
uint8_t I2CReadByte(uint8_t ack);

uint8_t I2CFindDevice(uint8_t addr);

#endif // I2C_H
