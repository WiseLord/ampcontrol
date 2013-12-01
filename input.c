#include <avr/io.h>
#include <avr/interrupt.h>

#include "input.h"

volatile uint8_t cmdBuf = CMD_NOCMD;	/* Command buffer, cleared when read */
volatile uint8_t rc5Buf = CMD_NOCMD;	/* Buffer for last command from RC */
volatile uint8_t rc5BufCnt = 0;			/* Counter for IR RC command buffer */

volatile uint16_t rc5Cmd; /**/
const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};
uint8_t rc5Cnt;

volatile uint16_t displayTime;

rc5State state = STATE_BEGIN;

void setDisplayTime(uint16_t value)
{
	displayTime = value;
}

uint16_t getDisplayTime(void)
{
	return displayTime;
}

void rc5Reset()
{
	rc5Cnt = 14;
	rc5Cmd = 0;
	state = STATE_BEGIN;
}

void rc5Init()
{
	MCUCR |= (1<<ISC10);	/* Set INT1 to trigger on any edge */
	BTN_DDR &= ~(1<<PD3);	/* Set PD3 (INT1) to input */
	TCCR1A = 0;				/* Reset Timer1 counter */
	TCCR1B = (1<<CS11);		/* Set Timer1 prescaler to 8 (2MHz) */
	GICR |= (1<<INT1);		/* Enable INT1 interrupt */

	rc5Reset();
}

ISR(INT1_vect)
{
	uint16_t delay = TCNT1;

	/* event: 0 / 2 - short space/pulse, 4 / 6 - long space/pulse */
	uint8_t event = (BTN_PIN & RC5_DATA) ? 2 : 0;

	if (delay > RC5_LONG_MIN && delay < RC5_LONG_MAX)
		event += 4;
	else if (delay < RC5_SHORT_MIN || delay > RC5_SHORT_MAX)
		rc5Reset();

	if (state == STATE_BEGIN) {
		rc5Cnt--;
		rc5Cmd |= (1 << rc5Cnt);
		state = STATE_MID1;
		TCNT1 = 0;
		return;
	}

	rc5State newstate = (trans[state] >> event) & 0x03;

	if (newstate == state || state > STATE_START0) {
		rc5Reset();
		return;
	}

	state = newstate;

	if (state == STATE_MID0)
		rc5Cnt--;
	else if (state == STATE_MID1) {
		rc5Cnt--;
		rc5Cmd |= 1 << rc5Cnt;
	}

	if (rc5Cnt == 0 && (state == STATE_START1 || state == STATE_MID0)) {
		/* If RC5 address is correct, send command to IR RC buffer */
		if ((rc5Cmd & RC5_ADDR_MASK) == RC5_ADDR) {
			rc5Buf = rc5Cmd & RC5_COMM_MASK;
			rc5BufCnt = 0;
		}
		rc5Reset();
	}

	TCNT1 = 0;
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
	static uint8_t cmdPrev = CMD_NOCMD;	/* Previous command */
	static int16_t btnCnt = 0;

	uint8_t btnPin = ~BTN_PIN;
	uint8_t encNow = btnPin & (ENC_A | ENC_B);
	uint8_t btnNow = btnPin & BTN_MASK;

	/* If encoder event has happened, send it to command buffer */
	switch (encNow) {
	case ENC_AB:
		if (encPrev == ENC_B) cmdBuf = CMD_VOL_UP;
		if (encPrev == ENC_A) cmdBuf = CMD_VOL_DOWN;
		break;
/*	case ENC_A:
		if (encPrev == ENC_AB) cmdBuf = COMM_ENC_UP;
		if (encPrev == ENC_0) cmdBuf = COMM_ENC_DOWN;
		break;
	case ENC_B:
		if (encPrev == ENC_0) cmdBuf = COMM_ENC_UP;
		if (encPrev == ENC_AB) cmdBuf = COMM_ENC_DOWN;
		break;
*/	case ENC_0:
		if (encPrev == ENC_A) cmdBuf = CMD_VOL_UP;
		if (encPrev == ENC_B) cmdBuf = CMD_VOL_DOWN;
		break;
	default:
		break;
	}
	encPrev = encNow;	/* Save current encoder state */

	uint8_t cmdNow = rc5Buf; /* Read current command from IR RC buffer */

	/* Clear IR RC buffer after 120ms (120 polls) */
	rc5BufCnt++;
	if (rc5BufCnt > 120) {
		rc5BufCnt = 120;
		rc5Buf = CMD_NOCMD;
	}

	/* If any button is pressed, read current command from buttons state */
	switch (btnNow) {
	case BTN_MENU:
		cmdNow = CMD_MENU;
		break;
	case BTN_RIGHT:
		cmdNow = CMD_VOL_UP;
		break;
	case BTN_LEFT:
		cmdNow = CMD_VOL_DOWN;
		break;
	case BTN_DOWN:
		cmdNow = CMD_TIME;
		break;
	case BTN_UP:
		cmdNow = CMD_SEARCH;
		break;
	default:
		break;
	}

	/* Send current command to buffer if it exists and differ from previous */
	if ((cmdNow != CMD_NOCMD) && (cmdPrev != cmdNow || btnCnt >= TIME_LONG))
		cmdBuf = cmdNow;

	/* Handle long press */
	if (btnCnt >= TIME_LONG) {
		switch (cmdNow) {
		case CMD_VOL_UP:
		case CMD_VOL_DOWN:
			btnCnt = TIME_LONG - TIME_REPEAT;
			break;
		default:
			btnCnt = 0;
			break;
		}
	}
	cmdPrev = cmdNow;

	if (cmdNow != CMD_NOCMD)
		btnCnt++;
	else
		btnCnt = 0;

	if (displayTime)
		displayTime--;
	return;
};

uint8_t getCommand(void) /* Read command and clear command buffer */
{
	uint8_t ret = cmdBuf;
	cmdBuf = CMD_NOCMD;
	return ret;
}
