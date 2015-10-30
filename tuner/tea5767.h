#ifndef TEA5767_H
#define TEA5767_H

#include <inttypes.h>

/* Control byte bits*/
#define TEA5767_CTRL_HCC		(1<<6)
#define TEA5767_CTRL_SNC		(1<<5)
#define TEA5767_CTRL_SMUTE		(1<<4)
#define TEA5767_CTRL_DTC		(1<<3)
#define TEA5767_CTRL_BL			(1<<2)
#define TEA5767_CTRL_PLLREF		(1<<1)
#define TEA5767_CTRL_XTAL		(1<<0)

#define TEA5767_I2C_ADDR		0b11000000

/* Write mode register values */

/* 0 register */
#define TEA5767_MUTE			(1<<7)	/* Mute output */
#define TEA5767_SM				(1<<6)	/* Search mode */
/* PLL 13..8 bits */

/* 1 register */
/* PLL 7..0 bits */

/* 2 register */
#define TEA5767_SUD				(1<<7)	/* Search Up(1) / Down(0) */
#define TEA5767_SSL_LOW			(1<<5)	/* Search stop level = 5 */
#define TEA5767_SSL_MID			(2<<5)	/* Search stop level = 7 */
#define TEA5767_SSL_HIGH		(3<<5)	/* Search stop level = 10 */
#define TEA5767_HLSI			(1<<4)	/* High(1) / Low(0) Side Injection */
#define TEA5767_MS				(1<<3)	/* Mono to stereo */
#define TEA5767_MR				(1<<2)	/* Mute Right */
#define TEA5767_ML				(1<<1)	/* Mute Left */
#define TEA5767_SWP1			(1<<0)	/* Software programmable port 1 high */

/* 3 register */
#define TEA5767_SWP2			(1<<7)	/* Software programmable port 2 high */
#define TEA5767_STBY			(1<<6)	/* Standby */
#define TEA5767_BL				(1<<5)	/* Band limit: Japan(1) / Europe(0) */
#define TEA5767_XTAL			(1<<4)	/* Clock frequency 32768 (1) */
#define TEA5767_SMUTE			(1<<3)	/* Soft mute */
#define TEA5767_HCC				(1<<2)	/* High cut control */
#define TEA5767_SNC				(1<<1)	/* Stereo Noise Cancelling */
#define TEA5767_SI				(1<<0)	/* Search indicator */

/* 4 register */
#define TEA5767_PLLREF			(1<<7)	/* 6.5MHz reference frequency */
#define TEA5767_DTC				(1<<6)	/* De-emphasis 75us(1) / 50us(0) */
/* Not used 5..0 bits */


/* Read mode register values */
/* 0 register */
#define TEA5767_RF				(1<<7)	/* Ready flag */
#define TEA5767_BLF				(1<<6)	/* Band limit flag*/
/* PLL 13..8 bits */

/* 1 register */
/* PLL 7..0 bits */

/* 2 register */
#define TEA5767_STEREO			(1<<7)	/* Stereo reception */
#define TEA5767_IF_CNT_MASK		0x7F	/* If counter result */

/* 3 register */
#define TEA5767_LEV_MASK		0xF0	/* ADC Level*/
#define TEA5767_CI				0x0F	/* Chip ID(0000)*/

/* 4 register */
#define TEA5767_RESERVED_MASK	0xFF	/* Not used */

#define TEA5767_BUF_READY(buf)	(buf[0] & TEA5767_RF)
#define TEA5767_BUF_STEREO(buf)	(buf[2] & TEA5767_STEREO)

void tea5767Init(uint8_t tea5767Ctrl);

void tea5767SetFreq(uint16_t freq, uint8_t mono);

uint8_t *tea5767ReadStatus(void);

void tea5767SetMute(uint8_t mute);

void tea5767PowerOn();
void tea5767PowerOff();

#endif /* TEA5767_H */
