#include "i2c.h"

#include <avr/io.h>

void I2CInit(void)
{
	TWBR = 18;
	TWSR = (1<<TWPS0);	/* Prescaler = 4, SCL=16000000/(16+2*18*4)=100000Hz */
	TWCR |= (1<<TWEN);	/* Enable TWI */
	return;
}

void I2CStart(uint8_t addr)
{
	uint8_t i = 0;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);	/* Start */
	while (!(TWCR & (1<<TWINT))) {				/* Wait for TWINT */
		if (i++ > 250)	/* Avoid endless loop */
			return;
	}

	I2CWriteByte(addr);

	return;
}

void I2CStop(void)
{
	uint8_t i = 0;
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);	/* Stop */
	while (TWCR & (1<<TWSTO)) {					/* Wait for TWSTO */
		if (i++ > 250)	/* Avoid endless loop */
			return;
	}
	return;
}

uint8_t I2CWriteByte(uint8_t data)
{
	uint8_t i = 0;
	TWDR = data;
	TWCR = (1<<TWEN) | (1<<TWINT);	/* Start data transfer */

	while (!(TWCR & (1<<TWINT))) {	/* Wait for finish */
		if (i++ > 250)	/* Avoid endless loop */
			return 1;
	}

	if (TWSR_STA != 0x18 && TWSR_STA != 0x28 && TWSR_STA != 0x40)
		return 1;
	return 0;
}

uint8_t I2CReadByte(uint8_t *data, uint8_t ack)
{
	uint8_t i = 0;
	if (ack)
		TWCR |= (1<<TWEA);
	else
		TWCR &= ~(1<<TWEA);
	TWCR |= (1 << TWINT);

	while (!(TWCR & (1<<TWINT))) {	/* Wait for finish */
		if (i++ > 250)	/* Avoid endless loop */
			return 1;
	}

	if (TWSR_STA != 0x58 && TWSR_STA != 0x50)
		return 1;

	*data = TWDR;
	return 0;
}
