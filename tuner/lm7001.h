#ifndef LM7001_H
#define LM7001_H

#include <avr/io.h>

#define LM7001_DDR			DDRC
#define LM7001_PORT			PORTC
#define LM7001_PIN			PINC

#define LM7001_DATA			(1<<PC2)
#define LM7001_CL			(1<<PC3)
#define LM7001_CE			(1<<PC4)

/* IF = 10.7MHz and step = 100kHz */
#define LM7001_IF			1070
#define LM7001_RF			10

/* Control byte */
#define LM7001_CTRL_B0		(1<<7)
#define LM7001_CTRL_B1		(1<<6)
#define LM7001_CTRL_B2		(1<<5)
#define LM7001_CTRL_TB		(1<<4)
#define LM7001_CTRL_R0		(1<<3)
#define LM7001_CTRL_R1		(1<<2)
#define LM7001_CTRL_R2		(1<<1)
#define LM7001_CTRL_S		(1<<0)

/* Set control byte to FM input with 100kHz Fref, (TB = 0, Bi = 0b000, Ri = 0b000, S = 1)*/
#define LM7001_CTRL_WORD	(LM7001_CTRL_S)

void lm7001Init(void);

void lm7001SetFreq(uint16_t freq);

#endif /* LM7001_H */
