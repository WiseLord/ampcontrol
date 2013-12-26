#ifndef INPUT_H
#define INPUT_H

#include <inttypes.h>

/* RC5 definitions */
#define RC5_DDR			DDRD
#define RC5_PIN			PIND
#define RC5_PORT		PORTD

#define RC5_DATA		(1<<PD3)

#define RC5_SHORT_MIN	888		/* 444 microseconds */
#define RC5_SHORT_MAX	2666	/* 1333 microseconds */
#define RC5_LONG_MIN	2668	/* 1334 microseconds */
#define RC5_LONG_MAX	4444	/* 2222 microseconds */

#define RC5_STBT_MASK	0x3000
#define RC5_TOGB_MASK	0x0800
#define RC5_ADDR_MASK	0x07C0
#define RC5_COMM_MASK	0x003F

#define RC5_ADDR		0x400


/* Buttons definitions */
#define BTN_DDR			DDRD
#define BTN_PIN			PIND
#define BTN_PORT		PORTD

#define BTN_MENU		(1<<PD7)
#define BTN_INPUT		(1<<PD4)
#define BTN_TIME		(1<<PD5)
#define BTN_MUTE		(1<<PD6)
#define BTN_STDBY		(1<<PD0)

#define BTN_MASK		(BTN_MENU | BTN_INPUT | BTN_TIME | BTN_MUTE | BTN_STDBY)

/* Encoder definitions */
#define ENC_DDR			DDRD
#define ENC_PIN			PIND
#define ENC_PORT		PORTD

#define ENC_A			(1<<PD2)
#define ENC_B			(1<<PD1)
#define ENC_AB			(ENC_A | ENC_B)
#define ENC_0			0

/* Standby/Mute/Fan port definitions */
#define SMBF_DDR DDRC
#define SMBF_PORT PORTC

#define FAN		(1<<PC2)
#define MUTE	(1<<PC5)
#define STDBY	(1<<PC6)
#define BCKL	(1<<PC7)

/* Remote control commands codes */
#define CMD_TIMER		0x26
#define CMD_STBY		0x0C

#define CMD_AV			0x38
#define CMD_TV			0x3F
#define CMD_MUTE		0x0D

#define CMD_NUM1		0x01
#define CMD_NUM2		0x02
#define CMD_NUM3		0x03
#define CMD_NUM4		0x04
#define CMD_NUM5		0x05
#define CMD_NUM6		0x06
#define CMD_NUM7		0x07
#define CMD_NUM8		0x08
#define CMD_NUM9		0x09
#define CMD_NUM0		0x00

#define CMD_PP			0x0E
#define CMD_DIG			0x0A

#define CMD_TXT			0x3C
#define CMD_MENU		0x3B

#define CMD_VOL_UP		0x10
#define CMD_VOL_DOWN	0x11
#define CMD_CH_UP		0x20
#define CMD_CH_DOWN		0x21

#define CMD_DESCR		0x2C
#define CMD_SCAN		0x2B
#define CMD_TIME		0x2A

#define CMD_SEARCH		0x1E
#define CMD_BLUE		0x34
#define CMD_STORE		0x29

#define CMD_RED			0x37
#define CMD_GREEN		0x37
#define CMD_YELLOW		0x32

#define CMD_NOCMD		0xFF

/* Handling long press actions */
#define TIME_LONG		800 /* Long press 800msec while 500 polls/sec */
#define TIME_REPEAT		100	/* Must be more than interval of RC5 sends */

typedef enum {
	STATE_START1,
	STATE_MID1,
	STATE_MID0,
	STATE_START0,
	STATE_ERROR,
	STATE_BEGIN,
	STATE_END
} rc5State;

void setDisplayTime(uint16_t value);
uint16_t getDisplayTime(void);

void rc5Init();
void rc5Reset();

void btnInit(void);
uint8_t getCommand(void);
uint8_t getCmdCount(void);

#endif /* INPUT_H */
