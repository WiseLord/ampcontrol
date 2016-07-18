#include "i2c.h"

#include <avr/io.h>

void I2CInit(void)
{
	// SCL = F_CPU / (16 + 2 * TWBR * prescaler)
	// SCL = 8000000 / (16 + 2 * 32 * 1)

	TWBR = 32;
	//TWSR = (0<<TWPS1) | (0<<TWPS0);				// Prescaler = 1

	TWCR |= (1<<TWEN);								// Enable TWI

	return;
}

void I2CStart(uint8_t addr)
{
	uint8_t i = 0;

	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);		// Start

	while(bit_is_clear(TWCR, TWINT)) {
		if (i++ > 250)								// Avoid endless loop
			return;
	}

	I2CWriteByte(addr);

	return;
}

void I2CStop(void)
{
	uint8_t i = 0;

	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);		// Stop

	while (bit_is_set(TWCR, TWSTO)) {				// Wait for TWSTO
		if (i++ > 250)								// Avoid endless loop
			break;
	}

	return;
}

void I2CWriteByte(uint8_t data)
{
	uint8_t i = 0;

	TWDR = data;
	TWCR = (1<<TWEN) | (1<<TWINT);					// Start data transfer

	while (bit_is_clear(TWCR, TWINT)) {				// Wait for finish
		if (i++ > 250)								// Avoid endless loop
			break;
	}

	return;
}

uint8_t I2CReadByte(uint8_t ack)
{
	uint8_t i = 0;

	if (ack)
		TWCR |= (1<<TWEA);
	else
		TWCR &= ~(1<<TWEA);

	TWCR |= (1 << TWINT);

	while (bit_is_clear(TWCR, TWINT)) {				// Wait for finish
		if (i++ > 250)								// Avoid endless loop
			break;
	}

	return TWDR;
}
