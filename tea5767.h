#ifndef TEA5767_H
#define TEA5767_H

#include <inttypes.h>

#define TEA5767_ADDR			0b11000000

/* Write mode register values */

/* 1 register */
#define TEA5767_MUTE			(1<<7)	/* Mute output */
#define TEA5767_SM				(1<<6)	/* Search mode */
/* PLL 13..8 bits */

/* 2 register */
/* PLL 7..0 bits */

/* 3 register */
#define TEA5767_SUD				(1<<7)	/* Search Up(1) / Down(0) */
#define TEA5767_SSL_LOW			(1<<5)	/* Search stop level = 5 */
#define TEA5767_SSL_MID			(2<<5)	/* Search stop level = 7 */
#define TEA5767_SSL_HIGH		(3<<5)	/* Search stop level = 10 */
#define TEA5767_HLSI			(1<<4)	/* High(1) / Low(0) Side Injection */
#define TEA5767_MS				(1<<3)	/* Mono to stereo */
#define TEA5767_MR				(1<<2)	/* Mute Right */
#define TEA5767_ML				(1<<1)	/* Mute Left */
#define TEA5767_SWP1			(1<<0)	/* Software programmable port 1 high */

/* 4 register */
#define TEA5767_SWP2			(1<<7)	/* Software programmable port 2 high */
#define TEA5767_STBY			(1<<6)	/* Standby */
#define TEA5767_BL				(1<<5)	/* Band limit: Japan(1) / Europe(0) */
#define TEA5767_XTAL			(1<<4)	/* Clock frequency 32768 (1) */
#define TEA5767_SMUTE			(1<<3)	/* Soft mute */
#define TEA5767_HCC				(1<<2)	/* High cut control */
#define TEA5767_SNC				(1<<1)	/* Stereo Noise Cancelling */
#define TEA5767_SI				(1<<0)	/* Search indicator */

/* 5 register */
#define TEA5767_PLLREF			(1<<7)	/* 6.5MHz reference frequency */
#define TEA5767_DTC				(1<<6)	/* De-emphasis 75us(1) / 50us(0) */
/* Not used 5..0 bits */


/* Read mode register values */
/* 1 register */
#define TEA5767_RF				(1<<7)	/* Ready flag */
#define TEA5767_BLF				(1<<6)	/* Band limit flag*/
/* PLL 13..8 bits */

/* 2 register */
/* PLL 7..0 bits */

/* 3 register */
#define TEA5767_STEREO			(1<<7)	/* Stereo reception */
#define TEA5767_IF_CNT_MASK		0x7F	/* If counter result */

/* 4 register */
#define TEA5767_LEV_MASK		0xF0	/* ADC Level*/
#define TEA5767_CI				0x0F	/* Chip ID(0000)*/

/* 5 register */
#define TEA5767_RESERVED_MASK	0xFF	/* Not used */

#define TEA5767_SEARCH_DOWN		0
#define SEARCH_UP		1

typedef struct {
	uint8_t high_cut;
	uint8_t st_noise;
	uint8_t soft_mute;
	uint8_t japan_band;
	uint8_t deemph_75;
	uint8_t pllref;
	uint8_t xtal_freq;
} tea5767Ctrl;

void tea5767Init(void);

void tea5767ReadStatus(uint8_t *buf);

uint8_t tea5767ADCLevel(uint8_t *buf);

uint8_t tea5767Stereo(uint8_t *buf);

uint8_t tea5767Ready(uint8_t *buf);

uint8_t tea5767BlReached (uint8_t *buf);

void tea5767SetOptimalFreq (uint32_t freq);

uint32_t tea5767FreqAvail(uint8_t *buf);

void tea5767Search(uint32_t freq, uint8_t *buf, uint8_t direction);

#endif // TEA5767_H
