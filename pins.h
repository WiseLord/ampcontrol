#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/* Standby/Mute port definitions */
#define STMU_MUTE			C
#define STMU_MUTE_LINE		(1<<5)

#define KS0066_D7			B
#define KS0066_D7_LINE		(1<<7)
#define KS0066_D6			B
#define KS0066_D6_LINE		(1<<6)
#define KS0066_D5			B
#define KS0066_D5_LINE		(1<<5)
#define KS0066_D4			B
#define KS0066_D4_LINE		(1<<4)
#define KS0066_D3			B
#define KS0066_D3_LINE		(1<<3)
#define KS0066_D2			B
#define KS0066_D2_LINE		(1<<2)
#define KS0066_D1			B
#define KS0066_D1_LINE		(1<<1)
#define KS0066_D0			B
#define KS0066_D0_LINE		(1<<0)
#define KS0066_BL			C
#define KS0066_BL_LINE		(1<<7)
#define KS0066_E			A
#define KS0066_E_LINE		(1<<4)
#define KS0066_RW			A
#define KS0066_RW_LINE		(1<<3)
#define KS0066_RS			A
#define KS0066_RS_LINE		(1<<2)

#define BACKLIGHT			C
#define BACKLIGHT_LINE		(1<<7)

/* RC5 definitions*/
#define RC5					D
#define RC5_LINE			(1<<3)

/* LM7001 definitions */
#define LM7001_DATA			C
#define LM7001_DATA_LINE	(1<<2)
#define LM7001_CL			C
#define LM7001_CL_LINE		(1<<3)
#define LM7001_CE			C
#define LM7001_CE_LINE		(1<<4)

#endif /* PINS_H */
