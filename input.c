#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"

static volatile uint8_t cmdBuf;		/* Command buffer, cleared when read */
static volatile int8_t encCnt;		/* Counter for encoder */

static volatile uint16_t displayTime;

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

	cmdBuf = CMD_EMPTY;
	encCnt = 0;
}

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

	if (btnNow) {
		if (btnNow == btnPrev) {
			btnCnt++;
			if (btnCnt == LONG_PRESS) {
				/* Place "long" command to buffer */
				switch (btnPrev) {
				case BTN_1:
					cmdBuf = CMD_BTN_1_LONG; /* Backlight on/off*/
					break;
				case BTN_2:
					cmdBuf = CMD_BTN_2_LONG; /* Switch input */
					break;
				case BTN_3:
					cmdBuf = CMD_BTN_3_LONG; /* Edit time */
					break;
				case BTN_4:
					cmdBuf = CMD_BTN_4_LONG; /* Loudness */
					break;
				case BTN_5:
					cmdBuf = CMD_BTN_5_LONG; /* Spectrum mode common/separate */
					break;
				case BTN_TESTMODE:
					cmdBuf = CMD_TESTMODE;
					break;
				}
			}
		} else {
			btnPrev = btnNow;
		}
	} else {
		if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS)) {
			/* Place "short" command to buffer */
			switch (btnPrev) {
			case BTN_1:
				cmdBuf = CMD_BTN_1; /* Standby */
				break;
			case BTN_2:
				cmdBuf = CMD_BTN_2; /* Switch input */
				break;
			case BTN_3:
				cmdBuf = CMD_BTN_3; /* Show time */
				break;
			case BTN_4:
				cmdBuf = CMD_BTN_4; /* Mute */
				break;
			case BTN_5:
				cmdBuf = CMD_BTN_5; /* Menu */
				break;
			}
		}
		/* Clear button counter */
		btnCnt = 0;
	}

	if (displayTime)
		displayTime--;

	return;
};

void setDisplayTime(uint8_t value)
{
	displayTime = value;
	displayTime <<= 10;
}

uint8_t getDisplayTime(void)
{
	return (displayTime + 1023) >> 10;
}

uint8_t getCommand(void)	/* Read command command buffer */
{
	uint8_t ret = cmdBuf;
	return ret;
}

void clearCommand(void)		/* Clear command buffer */
{
	cmdBuf = CMD_EMPTY;
	return;
}

int8_t getEncCnt(void)
{
	int8_t ret = encCnt;
	if (ret)
		cmdBuf = CMD_ENC;
	encCnt = 0;
	return ret;
}
