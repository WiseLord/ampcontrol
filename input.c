#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "remote.h"
#include "eeprom.h"

static volatile int8_t encCnt;
static volatile uint8_t cmdBuf;

/* Previous state */
static volatile uint16_t rc5SaveBuf;
static volatile uint8_t encPrev = ENC_0;
static volatile uint8_t btnPrev = BTN_STATE_0;

static volatile uint16_t dispTimer = 0;
static volatile uint16_t rtcTimer = 0;

//static uint8_t rcType;
static uint8_t rcAddr;
static uint8_t rcCode[CMD_RC_END];	/* Array with rc commands */

void rcCodesInit(void)
{
//	rcType = eeprom_read_byte((uint8_t*)EEPROM_RC_TYPE);
	rcAddr = eeprom_read_byte((uint8_t*)EEPROM_RC_ADDR);
	eeprom_read_block(rcCode, (uint8_t*)EEPROM_RC_CMD, CMD_RC_END);

	return;
}

void inputInit()
{
	/* Setup buttons and encoder as inputs with pull-up resistors */
	DDR(BUTTON_1) &= ~BUTTON_1_LINE;
	DDR(BUTTON_2) &= ~BUTTON_2_LINE;
	DDR(BUTTON_3) &= ~BUTTON_3_LINE;
	DDR(BUTTON_4) &= ~BUTTON_4_LINE;
	DDR(BUTTON_5) &= ~BUTTON_5_LINE;

	DDR(ENCODER_A) &= ~ENCODER_A_LINE;
	DDR(ENCODER_B) &= ~ENCODER_B_LINE;

	PORT(BUTTON_1) |= BUTTON_1_LINE;
	PORT(BUTTON_2) |= BUTTON_2_LINE;
	PORT(BUTTON_3) |= BUTTON_3_LINE;
	PORT(BUTTON_4) |= BUTTON_4_LINE;
	PORT(BUTTON_5) |= BUTTON_5_LINE;

	PORT(ENCODER_A) |= ENCODER_A_LINE;
	PORT(ENCODER_B) |= ENCODER_B_LINE;

	/* Set timer prescaller to 128 (62.5 kHz) and reset on match*/
	TCCR2 = ((1<<CS22) | (0<<CS21) | (1<<CS20) | (1<<WGM21));
	OCR2 = 62;						/* 62500/62 => 1008 polls/sec */
	TCNT2 = 0;						/* Reset timer value */
	TIMSK |= (1<<OCIE2);			/* Enable timer compare match interrupt */

	rcCodesInit();

	encCnt = 0;
	cmdBuf = CMD_END;
}

static CmdID rcCmdIndex(uint8_t rcCmd)
{
	CmdID i;

	for (i = 0; i < CMD_RC_END; i++)
		if (rcCmd == rcCode[i])
			return i;

	return CMD_RC_END;
}

ISR (TIMER2_COMP_vect)
{
	static int16_t btnCnt = 0;		/* Buttons press duration value */
	static uint16_t rcTimer;

	/* Current state */
	uint8_t encNow = ENC_0;
	uint8_t btnNow = BTN_STATE_0;

	if (~PIN(ENCODER_A) & ENCODER_A_LINE)
		encNow |= ENC_A;
	if (~PIN(ENCODER_B) & ENCODER_B_LINE)
		encNow |= ENC_B;

	if (~PIN(BUTTON_1) & BUTTON_1_LINE)
		btnNow |= BTN_1;
	if (~PIN(BUTTON_2) & BUTTON_2_LINE)
		btnNow |= BTN_2;
	if (~PIN(BUTTON_3) & BUTTON_3_LINE)
		btnNow |= BTN_3;
	if (~PIN(BUTTON_4) & BUTTON_4_LINE)
		btnNow |= BTN_4;
	if (~PIN(BUTTON_5) & BUTTON_5_LINE)
		btnNow |= BTN_5;

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
				case BTN_12:
					cmdBuf = CMD_BTN_12_LONG;
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
	IRData ir = takeIrData();

	CmdID rcCmdBuf = CMD_END;

	if (ir.ready && (/*ir.type == rcType && */ir.address == rcAddr)) {
		if (!ir.repeat || (rcTimer > RC_LONG_PRESS)) {
			rcTimer = 0;
			rcCmdBuf = rcCmdIndex(ir.command);
		}
		if (ir.command == rcCode[CMD_RC_VOL_UP] || ir.command == rcCode[CMD_RC_VOL_DOWN]) {
			if (rcTimer > RC_VOL_REPEAT) {
				rcTimer = RC_VOL_DELAY;
				rcCmdBuf = rcCmdIndex(ir.command);
			}
		}
	}

	if (cmdBuf == CMD_END)
		cmdBuf = rcCmdBuf;


	/* Time from last IR command */
	if (rcTimer < RC_PRESS_LIMIT)
		rcTimer++;

	// Current display mode timer
	if (dispTimer)
		dispTimer--;

	// RTC poll timer
	if (rtcTimer)
		rtcTimer--;

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
	cmdBuf = CMD_END;
	return ret;
}

uint16_t getRC5Buf(void)
{
	return rc5SaveBuf;
}

uint16_t getDispTimer(void)
{
	return dispTimer;
}

void setDispTimer(uint16_t value)
{
	dispTimer = value;

	return;
}

uint16_t getRtcTimer()
{
	return rtcTimer;
}

void setRtcTimer(uint16_t val)
{
	rtcTimer = val;

	return;
}
