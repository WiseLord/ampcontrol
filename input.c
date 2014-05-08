#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "rc5.h"

static volatile int8_t encCnt;
static volatile uint8_t btnCmdBuf;
static volatile uint8_t rc5CmdBuf;

static volatile uint16_t displayTime;
static volatile uint16_t rc5Timer;

static volatile uint8_t rc5DeviceAddr;
static volatile uint8_t rcCode[CMD_COUNT];	/* Array with rc5 commands */


void btnInit(void)
{
	/* Setup buttons and encoders as inputs with pull-up resistors */
	BTN_DDR &= ~(BTN_MASK);
	BTN_PORT |= BTN_MASK;

	ENC_DDR &= ~(ENC_AB);
	ENC_PORT |= ENC_AB;

	TCCR2 = 0b101;					/* Set timer prescaller to 128 (125 kHz) */
	OCR2 = 125;						/* 12500/125 => 1000 polls/sec */
	TCCR2 |= (1<<WGM21);			/* Reset counter on match */
	TCNT2 = 0;						/* Reset timer value */
	TIMSK |= (1<<OCIE2);			/* Enable timer compare match interrupt */

	/* Load RC5 device address and commands from eeprom */
	rc5DeviceAddr = eeprom_read_byte(eepromRC5Addr);
	uint8_t i;
	for (i = 0; i < CMD_COUNT; i++) {
		rcCode[i] = eeprom_read_byte(eepromRC5Cmd + i);
	}

	encCnt = 0;
	btnCmdBuf = CMD_BTN_EMPTY;
	rc5CmdBuf = CMD_RC5_EMPTY;
}
#include "ks0108.h"
#include <util/delay.h>
ISR (TIMER2_COMP_vect)
{
	static int16_t btnCnt = 0;		/* Buttons press duration value */

	/* Previous state */
	static uint8_t encPrev = ENC_0;
	static uint8_t btnPrev = 0;
	/* Current state */
	uint8_t encNow = ~ENC_PIN & ENC_AB;
	uint8_t btnNow = ~BTN_PIN & BTN_MASK;

	/* If encoder event has happened, inc/dec encoder counter */
	switch (encNow) {
	case ENC_AB:
		if (encPrev == ENC_B)
			encCnt++;
		if (encPrev == ENC_A)
			encCnt--;
		break;
/*	case ENC_A:
		if (encPrev == ENC_AB)
			encCnt++;
		if (encPrev == ENC_0)
			encCnt--;
		break;
	case ENC_B:
		if (encPrev == ENC_0)
			encCnt++;
		if (encPrev == ENC_AB)
			encCnt++;
		break;
	case ENC_0:
		if (encPrev == ENC_A)
			encCnt++;
		if (encPrev == ENC_B)
			encCnt++;
		break;
*/	}
	encPrev = encNow;				/* Save current encoder state */

	/* If button event has happened, place it to command buffer */
	if (btnNow) {
		if (btnNow == btnPrev) {
			btnCnt++;
			if (btnCnt == LONG_PRESS) {
				switch (btnPrev) {
				case BTN_1:
					btnCmdBuf = CMD_BTN_1_LONG;
					break;
				case BTN_2:
					btnCmdBuf = CMD_BTN_2_LONG;
					break;
				case BTN_3:
					btnCmdBuf = CMD_BTN_3_LONG;
					break;
				case BTN_4:
					btnCmdBuf = CMD_BTN_4_LONG;
					break;
				case BTN_5:
					btnCmdBuf = CMD_BTN_5_LONG;
					break;
				case BTN_TEST_INPUT:
					btnCmdBuf = CMD_BTN_TESTMODE;
					break;
				}
			}
		} else {
			btnPrev = btnNow;
		}
	} else {
		if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS)) {
			switch (btnPrev) {
			case BTN_1:
				btnCmdBuf = CMD_BTN_1;
				break;
			case BTN_2:
				btnCmdBuf = CMD_BTN_2;
				break;
			case BTN_3:
				btnCmdBuf = CMD_BTN_3;
				break;
			case BTN_4:
				btnCmdBuf = CMD_BTN_4;
				break;
			case BTN_5:
				btnCmdBuf = CMD_BTN_5;
				break;
			}
		}
		btnCnt = 0;
	}

	/* Place RC5 event to command buffer if enough RC5 timer ticks */
	uint16_t rc5Cmd = getRC5RawBuf();
//	static uint8_t togBitNow = 0;
//	static uint8_t togBitPrev = 0;
	uint8_t i;


	rc5Cmd &= RC5_COMM_MASK;

		for (i = 0; i < CMD_COUNT; i++) {
			if (rc5Cmd == rcCode[i]) {
				rc5CmdBuf = i;
				break;
			}
		}

/*
	if ((rc5Cmd & RC5_ADDR_MASK) >> 6 == rc5DeviceAddr) {
		if (rc5Cmd & RC5_TOGB_MASK)
			togBitNow = 1;
		else
			togBitNow = 0;
		rc5Cmd &= RC5_COMM_MASK;
		if ((togBitNow != togBitPrev) ||
			((rc5Timer > 200) &
			 (rc5Cmd == rcCode[CMD_RC5_VOL_UP] ||
			  rc5Cmd == rcCode[CMD_RC5_VOL_DOWN])) ||
			(rc5Timer > 800)) {
			encCnt++;
			rc5Timer = 0;
			rc5CmdBuf = CMD_RC5_EMPTY;
			for (i = 0; i < CMD_COUNT; i++) {
				if (rc5Cmd == rcCode[i])
				{
					rc5CmdBuf = i;
					break;
				}
			}
		}
		togBitPrev = togBitNow;
	}
*/

	/* Timer of current display mode */
	if (displayTime)
		displayTime--;

	/* Time from last IR command */
	if (rc5Timer < 1000)
		rc5Timer++;
	return;
};


int8_t getEncoder(void)
{
	int8_t ret = encCnt;
	encCnt = 0;
	return ret;
}

uint8_t getBtnCmd(void)
{
	uint8_t ret = btnCmdBuf;
	btnCmdBuf = CMD_BTN_EMPTY;
	return ret;
}

uint8_t getRC5Cmd(void)
{
	uint8_t ret = rc5CmdBuf;
	rc5CmdBuf = CMD_RC5_EMPTY;
	return ret;
}


void setDisplayTime(uint8_t value)
{
	displayTime = value;
	displayTime <<= 10;
}

uint8_t getDisplayTime(void)
{
	return (displayTime | 0x3F) >> 10;
}
