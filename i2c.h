#ifndef I2C_H
#define I2C_H

#include <inttypes.h>

#define TWSR_STA	(TWSR & 0xF8)

void I2CInit(void);
void I2CStart(void);
void I2CStop(void);

uint8_t I2CWriteByte(uint8_t data);
uint8_t I2CReadByte(uint8_t *data, uint8_t ack);

uint8_t I2CWrite(uint8_t device, uint8_t address, uint8_t data);
uint8_t I2CRead(uint8_t device, uint8_t address, uint8_t *data);

uint8_t I2CWrComm(uint8_t device, uint8_t command);

#endif /* I2C_H */
