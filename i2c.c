#include "i2c.h"

#include <avr/io.h>

void I2CInit()
{
    // SCL = F_CPU / (16 + 2 * TWBR * prescaler)
    // SCL = 16000000 / (16 + 2 * 18 * 4)

    TWBR = 18;
    TWSR = (0 << TWPS1) | (1 << TWPS0);                 // Prescaler = 4

    TWCR |= (1 << TWEN);                                // Enable TWI
}

uint8_t I2CStart(uint8_t addr)
{
    uint8_t i = 0;

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);   // Start

    while (bit_is_clear(TWCR, TWINT)) {
        if (i++ > 250)                                  // Avoid endless loop
            return 0;
    }

    return I2CWriteByte(addr);
}

void I2CStop()
{
    uint8_t i = 0;

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);   // Stop

    while (bit_is_set(TWCR, TWSTO)) {                   // Wait for TWSTO
        if (i++ > 250)                                  // Avoid endless loop
            break;
    }
}

uint8_t I2CWriteByte(uint8_t data)
{
    uint8_t i = 0;

    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);                  // Start data transfer

    while (bit_is_clear(TWCR, TWINT)) {                 // Wait for finish
        if (i++ > 250)                                  // Avoid endless loop
            break;
    }

    return TWSR & 0xF8;
}

uint8_t I2CReadByte(uint8_t ack)
{
    uint8_t i = 0;

    if (ack)
        TWCR |= (1 << TWEA);
    else
        TWCR &= ~(1 << TWEA);

    TWCR |= (1 << TWINT);

    while (bit_is_clear(TWCR, TWINT)) {                 // Wait for finish
        if (i++ > 250)                                  // Avoid endless loop
            break;
    }

    return TWDR;
}
