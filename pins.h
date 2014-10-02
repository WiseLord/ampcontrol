#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/* KS0108 Data port*/
#define KS0108_D0			B
#define KS0108_D0_LINE		(1<<0)
#define KS0108_D1			B
#define KS0108_D1_LINE		(1<<1)
#define KS0108_D2			B
#define KS0108_D2_LINE		(1<<2)
#define KS0108_D3			B
#define KS0108_D3_LINE		(1<<3)
#define KS0108_D4			B
#define KS0108_D4_LINE		(1<<4)
#define KS0108_D5			B
#define KS0108_D5_LINE		(1<<5)
#define KS0108_D6			B
#define KS0108_D6_LINE		(1<<6)
#define KS0108_D7			B
#define KS0108_D7_LINE		(1<<7)
/* KS0108 control port*/
#define KS0108_DI			A
#define KS0108_DI_LINE		(1<<2)
#define KS0108_RW			A
#define KS0108_RW_LINE		(1<<3)
#define KS0108_E			A
#define KS0108_E_LINE		(1<<4)
#define KS0108_CS1			A
#define KS0108_CS1_LINE		(1<<5)
#define KS0108_CS2			A
#define KS0108_CS2_LINE		(1<<6)
#define KS0108_RES			A
#define KS0108_RES_LINE		(1<<7)
/* KS0108 Backlight port */
#define KS0108_BCKL			C
#define KS0108_BCKL_LINE	(1<<7)

/* ST7920 Data port*/
#define ST7920_D0			B
#define ST7920_D0_LINE		(1<<0)
#define ST7920_D1			B
#define ST7920_D1_LINE		(1<<1)
#define ST7920_D2			B
#define ST7920_D2_LINE		(1<<2)
#define ST7920_D3			B
#define ST7920_D3_LINE		(1<<3)
#define ST7920_D4			B
#define ST7920_D4_LINE		(1<<4)
#define ST7920_D5			B
#define ST7920_D5_LINE		(1<<5)
#define ST7920_D6			B
#define ST7920_D6_LINE		(1<<6)
#define ST7920_D7			B
#define ST7920_D7_LINE		(1<<7)
/* ST7920 control port*/
#define ST7920_RS			A
#define ST7920_RS_LINE		(1<<2)
#define ST7920_RW			A
#define ST7920_RW_LINE		(1<<3)
#define ST7920_E			A
#define ST7920_E_LINE		(1<<4)
#define ST7920_PSB			A
#define ST7920_PSB_LINE		(1<<5)
#define ST7920_RST			A
#define ST7920_RST_LINE		(1<<7)
/* ST7920 Backlight port */
#define ST7920_BCKL			C
#define ST7920_BCKL_LINE	(1<<7)

/* RC5 definitions*/
#define RC5					D
#define RC5_LINE			(1<<3)

/* Standby/Mute port definitions */
#define STMU_MUTE			C
#define STMU_MUTE_LINE		(1<<5)
#define STMU_STBY			C
#define STMU_STBY_LINE		(1<<6)

/* LM7001 definitions */
#define LM7001_DATA			C
#define LM7001_DATA_LINE	(1<<2)
#define LM7001_CL			C
#define LM7001_CL_LINE		(1<<3)
#define LM7001_CE			C
#define LM7001_CE_LINE		(1<<4)

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
