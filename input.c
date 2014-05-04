#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"

volatile uint8_t cmdBuf = CMD_EMPTY;	/* Command buffer, cleared when read */
volatile uint8_t encCnt = 0;			/* Counter for encoder */

volatile uint16_t displayTime;

void setDisplayTime(uint16_t value)
{
	displayTime = value;
}

uint16_t getDisplayTime(void)
{
	return displayTime;
}

void btnInit(void)
{
	/* Setup buttons and encoders as inputs with pull-up resistors */
	BTN_DDR &= ~(BTN_MASK);
	BTN_PORT |= BTN_MASK;

	ENC_DDR &= ~(ENC_AB);
	ENC_PORT |= ENC_AB;

	TCCR2 = 0b101;			/* Set timer prescaller to 128 (125 kHz) */
	OCR2 = 125;				/* 12500/125 => 1000 polls/sec */
	TCCR2 |= (1<<WGM21);	/* Reset counter on match */
	TCNT2 = 0;				/* Reset timer value */
	TIMSK |= (1<<OCIE2);	/* Enable timer compare match interrupt */
}

ISR (TIMER2_COMP_vect) {
	static uint8_t encPrev = ENC_0;		/* Previous encoder state */
	static int16_t btnCnt = 0;

	static uint8_t btnPrev = 0;

	uint8_t btnPin = ~BTN_PIN;
	uint8_t encNow = btnPin & (ENC_A | ENC_B);
	uint8_t btnNow = btnPin & BTN_MASK;

	/* If encoder event has happened, send it to command buffer */
	switch (encNow) {
	case ENC_AB:
		if (encPrev == ENC_B) {
			cmdBuf = CMD_VOL_UP;
			encCnt++;
		}
		if (encPrev == ENC_A) {
			cmdBuf = CMD_VOL_DOWN;
			encCnt++;
		}
		break;
/*	case ENC_A:
		if (encPrev == ENC_AB) {
			cmdBuf = CMD_VOL_UP;
			encCnt++;
		}
		if (encPrev == ENC_0) {
			cmdBuf = CMD_VOL_DOWN;
			encCnt++;
		}
		break;
	case ENC_B:
		if (encPrev == ENC_0) {
			cmdBuf = CMD_VOL_UP;
			encCnt++;
		}
		if (encPrev == ENC_AB) {
			cmdBuf = CMD_VOL_DOWN;
			encCnt++;
		}
		break;
	case ENC_0:
		if (encPrev == ENC_A) {
			cmdBuf = CMD_VOL_UP;
			encCnt++;
		}
		if (encPrev == ENC_B) {
			cmdBuf = CMD_VOL_DOWN;
			encCnt++;
		}
		break;
*/	default:
		break;
	}
	encPrev = encNow;	/* Save current encoder state */

	if (btnNow) {
		if (btnNow == btnPrev) {
			btnCnt++;
			if (btnCnt == LONG_PRESS) {
				/* Place "long" command to buffer */
				switch (btnPrev) {
				case BTN_STDBY:
					cmdBuf = CMD_BACKLIGHT; /* Backlight on/off*/
					break;
				case BTN_MENU:
					cmdBuf = CMD_SP_MODE; /* Spectrum mode common/separate */
					break;
				case BTN_MUTE:
					cmdBuf = CMD_LOUDNESS; /* Loudness */
					break;
				case BTN_TIME:
					cmdBuf = CMD_EDIT_TIME; /* Edit time */
					break;
				case BTN_INPUT:
					cmdBuf = CMD_NEXT_INPUT; /* Switch input */
					break;
				case BTN_TESTMODE:
					cmdBuf = CMD_TESTMODE;
					break;
				default:
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
			case BTN_STDBY:
				cmdBuf = CMD_STBY; /* Standby */
				break;
			case BTN_MENU:
				cmdBuf = CMD_MENU; /* Menu */
				break;
			case BTN_MUTE:
				cmdBuf = CMD_MUTE; /* Mute */
				break;
			case BTN_TIME:
				cmdBuf = CMD_TIME; /* Show time */
				break;
			case BTN_INPUT:
				cmdBuf = CMD_NEXT_INPUT; /* Switch input */
				break;
			default:
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

uint8_t getCommand(void) /* Read command and clear command buffer */
{
	uint8_t ret = cmdBuf;
	cmdBuf = CMD_EMPTY;
	return ret;
}

uint8_t getCmdCount(void)
{
	uint8_t ret = encCnt;
	encCnt = 0;
	return ret;
}
