#ifndef RDA5807M_H
#define RDA5807M_H

#include <inttypes.h>

#define RDA5807M_ADDR				0b00100000

/* Write mode register values */

/* 0 register (02H) */
#define RDA5807_DHIZ				(1<<7) /* Audio out enable (1) / High impedance (0)*/
#define RDA5807_DMUTE				(1<<6) /* Audio unmuted (1) / muted (0) */
#define RDA5807_MONO				(1<<5) /* Mono mode (1) / stereo mode (0) */
#define RDA5807_BASS				(1<<4) /* Bass boost (1) */
#define RDA5807_RCLK_NON_CAL_MODE	(1<<3) /* RCLK always on (0) */
#define RDA5807_RCLK_DIR_IN_MODE	(1<<2) /* RCLK direct input mode (1) */
#define RDA5807_SEEKUP				(1<<1) /* Seek up (0) / down (1) */
#define RDA5807_SEEK				(1<<0) /* Stop seek (0) / start seek in SEEKUP direction (1) */

/* 1 register (02L) */
#define RDA5807_SKMODE				(1<<7) /* Stop seeking on band limit (1) */
#define RDA5807_CLK_MODE_32768		(0<<4) /* Select quartz frequency */
	#define RDA5807_CLK_MODE_12M	(1<<4)
	#define RDA5807_CLK_MODE_24M	(5<<4)
	#define RDA5807_CLK_MODE_13M	(2<<4)
	#define RDA5807_CLK_MODE_26M	(6<<4)
	#define RDA5807_CLK_MODE_19M2	(3<<4)
	#define RDA5807_CLK_MODE_38M4	(7<<4)
#define RDA5807_RDS_EN				(1<<3) /* RDS/RBDS enable (1) */
#define RDA5807_NEW_METHOD			(1<<2) /* New demodulation method (1) */
#define RDA5807_SOFT_RESET			(1<<1) /* Reset settings (1) */
#define RDA5807_ENABLE				(1<<0) /* Power on radio (1) */

/* 2 register (03H) */
/* CHAN 9..2 bits */
/*
 * BAND = 0    => Freq = Channel spacing (kHz) * CHAN + 87.0MHz
 * BAND = 1,2  => Freq = Channel spacing (kHz) * CHAN + 76.0MHz
 * BAND = 3    => Freq = Channel spacing (kHz) * CHAN + 65.0MHz
*/

/* 3 register (03L) */
/* CHAN 1.. 0 bits */
#define RDA5807_DIRECT_MODE			(1<<5) /* Direct mode (1), only used when test */
#define RDA5807_TUNE				(1<<4) /* Tune enable (1) */
#define RDA5807_BAND_US_EUROPE		(0<<2) /* 87..108 MHz */
	#define RDA5807_BAND_JAPAN		(1<<2) /* 76..97 MHz */
	#define RDA5807_BAND_WORLDWIDE	(2<<2) /* 76..108 MHz */
	#define RDA5807_BAND_EASTEUROPE	(3<<2) /* 65..76 MHz */
#define RDA5807_SPACE_100			(0<<0) /* 100kHz step */
	#define RDA5807_SPACE_200		(1<<0) /* 200kHz step */
	#define RDA5807_SPACE_50		(2<<0) /* 50kHz step */
	#define RDA5807_SPACE_25		(3<<0) /* 25kHz step */

/* 4 register (04H) */
#define RDA5807_DE					(1<<3) /* De-emphasis 75us (0) / 50us (1) */
#define RDA5807_SOFTMUTE_EN			(1<<1) /* Softmute enable (1) */
#define RDA5807_AFCD				(1<<0) /* AFC disable (1) */

/* 5 register (04L) */
/* Unused on RDA5807M, used on RDA5807FP for GPIO/I2S control */

/* 6 register (05H) */
#define RDA5807_INT_MODE			(1<<7) /* 5ms interrupt for RDSIEN on RDS ready (0) */
#define RDA5807_SEEKTH				0x0F   /* Seek SNR threshold, 4bits, default 1000=32dB */

/* 7 register (05L) */
#define RDA5807_LNA_PORT_SEL		0xC0   /* Only for RDA5807FP 2 bit (10) to select FMIN input */
#define RDA5807_VOLUME				0x0F   /* 4 bits volume (0000 - muted, 1111 - max) */

/* 8 register (06H) */
#define RDA5807_OPEN_MODE			0xC0   /* 2 bits (11) to open read-only regs for writing */
/* Other bits for RDA5807FP, for I2S control */

/* 9 register (06L) */
/* It's for RDA5807FP, for I2S control */

/* 10 register (07H) */
#define RDA5807_TH_SOFRBLEND		0x7C   /* 5 bits for noise soft blend, default 10000 */
#define RDA5807_65M_50M_MODE		(1<<1) /* For BAND=11, 50..76MHz (0) */

/* 11 register (07L) */
#define RDA5807_TH_OLD				0xFC   /* 6 bits seek treshold, valid for SKMODE=1 */
#define RDA5807_SOFTBLEND_EN		(1<<1) /* Softblend enable (1) */
#define RDA5807_FREQ_MODE			(1<<0) /* Manual freq setup (1) for 12,13 regs */

/* 12,13 registers (08H,L) */
/* Manual frequency value. Freq = 76000(87000)kHz + value of 08 word */

/* Read mode register values */

/* 0 register (0AH) */
#define RDA5807_RDSR				(1<<7) /* RDS ready (1) */
#define RDA5807_STC					(1<<6) /* Seek/tune complete (1) */
#define RDA5807_SF					(1<<5) /* Seek failure (1) */
#define RDA5807_RDSS				(1<<4) /* RDS decoder synchronized (1) */
#define RDA5807_BLK_E				(1<<3) /* (When RDS enabled) block E has been found (1)*/
#define RDA5807_ST					(1<<2) /* Stereo indicator (1) */
/* READCHAN 9,8 bits */

/* 1 register (0AL) */
/* READCHAN 7-0 bits */

/* 2 register (0BH) */
#define RDA5807_RSSI				0xFE   /* 7 bits of RSSI signal level*/
#define RDA5807_FM_TRUE				(1<<0) /* Current channel is a station (1) */

/* 3 register (0BL) */
#define RDA5807_FM_READY			(1<<7) /* Ready */
#define RDA5807_ABCD_E				(1<<4) /* Data block E (1) or blocks A-D (0) */
#define RDA5807_BLERA				0x0C   /* 2 bits error level in block A(RDS) or E(RBDS) */
#define RDA5807_BLERB				0x03   /* 2 bits error level in block B(RDS) or E(RBDS) */

/* 4-11 registers */
/* RDS data registers:
 * 4-5   => A,
 * 6-7   => B,
 * 8-9   => C,
 * 10-11 => D,
 * or 4-11 => E when ABCD_E = 1
 */

#define RDA5807_BUF_READY(buf)	(buf[3] & RDA5807_FM_READY)
#define RDA5807_BUF_STEREO(buf)	(buf[0] & RDA5807_ST)

void rda5807Init(void);
void rda5807SetFreq(uint16_t freq, uint8_t mono);
void rda5807ReadStatus(uint8_t *buf);

#endif /* RDA5807M_H */
