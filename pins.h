#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/* KS0066 Data port*/
#define KS0066_D4			B
#define KS0066_D4_LINE		(1<<4)
#define KS0066_D5			B
#define KS0066_D5_LINE		(1<<5)
#define KS0066_D6			B
#define KS0066_D6_LINE		(1<<6)
#define KS0066_D7			B
#define KS0066_D7_LINE		(1<<7)
/* KS0066 control port*/
#define KS0066_RS			B
#define KS0066_RS_LINE		(1<<0)
#define KS0066_E			B
#define KS0066_E_LINE		(1<<2)

/* KS0066 Backlight port */
#define BCKL				B
#define BCKL_LINE			(1<<3)

/* RC5 definitions*/
#define RC5					D
#define RC5_LINE			(1<<3)

/* Standby/Mute port definitions */
#define STMU_MUTE			B
#define STMU_MUTE_LINE		(1<<1)

/* LM7001 definitions */
#define LM7001_DATA			C
#define LM7001_DATA_LINE	(1<<3)
#define LM7001_CL			C
#define LM7001_CL_LINE		(1<<2)
#define LM7001_CE			C
#define LM7001_CE_LINE		(1<<1)

/* Encoder definitions */
#define ENCODER_A			D
#define ENCODER_A_LINE		(1<<2)
#define ENCODER_B			D
#define ENCODER_B_LINE		(1<<1)

/* Buttons definitions */
#define BUTTON_1			D
#define BUTTON_1_LINE		(1<<0)
#define BUTTON_2			D
#define BUTTON_2_LINE		(1<<4)
#define BUTTON_3			D
#define BUTTON_3_LINE		(1<<5)
#define BUTTON_4			D
#define BUTTON_4_LINE		(1<<6)
#define BUTTON_5			D
#define BUTTON_5_LINE		(1<<7)

#endif /* PINS_H */
