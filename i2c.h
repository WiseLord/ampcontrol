#ifndef I2C_H
#define I2C_H

#include <inttypes.h>

void I2CInit(void);
void I2CStart(void);
void I2CStop(void);
uint8_t I2CWriteByte(uint8_t data);
uint8_t I2CReadByte(uint8_t *data, uint8_t ack);
uint8_t DS1307Write(uint8_t address, uint8_t data);
uint8_t DS1307Read(uint8_t address, uint8_t *data);

void showTime(void);

#endif // I2C_H
