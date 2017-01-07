#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/* KS0066 Data port*/
#define KS0066_D4			D
#define KS0066_D4_LINE		(1<<7)
#define KS0066_D5			B
#define KS0066_D5_LINE		(1<<0)
#define KS0066_D6			B
#define KS0066_D6_LINE		(1<<1)
#define KS0066_D7			B
#define KS0066_D7_LINE		(1<<2)
/* KS0066 control port*/
#define KS0066_RS			D
#define KS0066_RS_LINE		(1<<5)
#define KS0066_E			D
#define KS0066_E_LINE		(1<<6)

/* Backlight port */
#define BCKL				C
#define BCKL_LINE			(1<<3)

/* RC5 definitions*/
#define RC					D
#define RC_LINE				(1<<3)

/* AVR ISP definitions */
#define ISP_MOSI			B
#define ISP_MOSI_LINE		(1<<3)
#define ISP_MISO			B
#define ISP_MISO_LINE		(1<<4)
#define ISP_SCK				B
#define ISP_SCK_LINE		(1<<5)

/* LM7001 definitions */
#define LM7001_DATA			ISP_MOSI
#define LM7001_DATA_LINE	ISP_MOSI_LINE
#define LM7001_CE			ISP_MISO
#define LM7001_CE_LINE		ISP_MISO_LINE
#define LM7001_CL			ISP_SCK
#define LM7001_CL_LINE		ISP_SCK_LINE

/* PGA2310 definitions */
#define PGA2310_SDI			ISP_MOSI
#define PGA2310_SDI_LINE	ISP_MOSI_LINE
#define PGA2310_CS			ISP_MISO
#define PGA2310_CS_LINE		ISP_MISO_LINE
#define PGA2310_SCLK		ISP_SCK
#define PGA2310_SCLK_LINE	ISP_SCK_LINE

/* Standby/Mute port definitions */
#define STMU_STBY			B
#define STMU_STBY_LINE		(1<<7)

/* Encoder definitions */
#define ENCODER_A			C
#define ENCODER_A_LINE		(1<<0)
#define ENCODER_B			C
#define ENCODER_B_LINE		(1<<1)

/* Buttons definitions */
#define BUTTON_1			B
#define BUTTON_1_LINE		(1<<6)
#define BUTTON_2			D
#define BUTTON_2_LINE		(1<<4)
#define BUTTON_3			D
#define BUTTON_3_LINE		(1<<2)
#define BUTTON_4			D
#define BUTTON_4_LINE		(1<<1)
#define BUTTON_5			D
#define BUTTON_5_LINE		(1<<0)

/* LM7001 definitions */
#define LC72131_DI			ISP_MOSI
#define LC72131_DI_LINE		ISP_MOSI_LINE
#define LC72131_CL			ISP_SCK
#define LC72131_CL_LINE		ISP_SCK_LINE
#define LC72131_CE			ISP_MISO
#define LC72131_CE_LINE		ISP_MISO_LINE

#endif /* PINS_H */
