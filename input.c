#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "rc5.h"
#include "eeprom.h"

static volatile int8_t encCnt;
static volatile uint8_t cmdBuf;
static volatile uint16_t rc5SaveBuf;

static volatile uint16_t displayTime;

static uint8_t rc5DeviceAddr;
static uint8_t rcCode[RC5_CMD_COUNT];		/* Array with rc5 commands */

void inputInit()
{
	uint8_t i;

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
	for (i = 0; i < RC5_CMD_COUNT; i++) {
		rcCode[i] = eeprom_read_byte(eepromRC5Cmd + i);
	}

	encCnt = 0;
	cmdBuf = CMD_EMPTY;
}

static uint8_t rc5CmdIndex(uint8_t rc5Cmd)
{
	uint8_t i;

	for (i = 0; i < RC5_CMD_COUNT; i++)
		if (rc5Cmd == rcCode[i])
			return i;

	return CMD_EMPTY;
}

ISR (TIMER2_COMP_vect)
{
	static int16_t btnCnt = 0;		/* Buttons press duration value */
	static uint16_t rc5Timer;

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
					cmdBuf = CMD_BTN_1_LONG;
					break;
				case BTN_2:
					cmdBuf = CMD_BTN_2_LONG;
					break;
				case BTN_3:
					cmdBuf = CMD_BTN_3_LONG;
					break;
				case BTN_4:
					cmdBuf = CMD_BTN_4_LONG;
					break;
				case BTN_5:
					cmdBuf = CMD_BTN_5_LONG;
					break;
				case BTN_TEST_INPUT:
					cmdBuf = CMD_BTN_TESTMODE;
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
				cmdBuf = CMD_BTN_1;
				break;
			case BTN_2:
				cmdBuf = CMD_BTN_2;
				break;
			case BTN_3:
				cmdBuf = CMD_BTN_3;
				break;
			case BTN_4:
				cmdBuf = CMD_BTN_4;
				break;
			case BTN_5:
				cmdBuf = CMD_BTN_5;
				break;
			}
		}
		btnCnt = 0;
	}

	/* Place RC5 event to command buffer if enough RC5 timer ticks */
	uint16_t rc5Buf = getRC5RawBuf();
	if (rc5Buf != RC5_BUF_EMPTY)
		rc5SaveBuf = rc5Buf;


	static uint8_t togBitNow = 0;
	static uint8_t togBitPrev = 0;

	uint8_t rc5CmdBuf = CMD_EMPTY;
	uint8_t rc5Cmd;

	if ((rc5Buf != RC5_BUF_EMPTY) && ((rc5Buf & RC5_ADDR_MASK) >> 6 == rc5DeviceAddr)) {
		if (rc5Buf & RC5_TOGB_MASK)
			togBitNow = 1;
		else
			togBitNow = 0;

		rc5Cmd = rc5Buf & RC5_COMM_MASK;
		if ((togBitNow != togBitPrev) || (rc5Timer > 800)) {
			rc5Timer = 0;
			rc5CmdBuf = rc5CmdIndex(rc5Cmd);
		}
		if (rc5Cmd == rcCode[CMD_RC5_VOL_UP] || rc5Cmd == rcCode[CMD_RC5_VOL_DOWN]) {
			if (rc5Timer > 400) {
				rc5Timer = 360;
				rc5CmdBuf = rc5CmdIndex(rc5Cmd);
			}
		}
		togBitPrev = togBitNow;
	}

	if (cmdBuf == CMD_EMPTY) {
		cmdBuf = rc5CmdBuf;
	}

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
	uint8_t ret = cmdBuf;
	cmdBuf = CMD_EMPTY;
	return ret;
}

uint16_t getRC5Buf(void)
{
	return rc5SaveBuf;
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
