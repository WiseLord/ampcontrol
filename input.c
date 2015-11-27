#include "input.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "rc5.h"
#include "eeprom.h"

static volatile int8_t encCnt;
static volatile cmdID cmdBuf;
static volatile int8_t encRes;
static volatile uint8_t silenceTime;

/* Previous state */
static volatile uint16_t rc5SaveBuf;
static volatile uint8_t btnSaveBuf;
static volatile uint8_t encPrev = ENC_0;
static volatile uint8_t btnPrev = BTN_STATE_0;

static volatile uint16_t displayTime;

static volatile uint16_t sensTimer;					/* Timer of temperature measuring process */
static volatile int16_t stbyTimer = STBY_TIMER_OFF;	/* Standby timer */
static volatile uint16_t secTimer;					/* 1 second timer */
static volatile uint8_t clockTimer;
static volatile int16_t silenceTimer;				/* Timer to check silence */

static uint8_t rc5DeviceAddr;
static uint8_t rcCode[RC5_CMD_COUNT];				/* Array with rc5 commands */

void rc5CodesInit(void)
{
	uint8_t i;

	rc5DeviceAddr = eeprom_read_byte((uint8_t*)EEPROM_RC5_ADDR);

	for (i = 0; i < RC5_CMD_COUNT; i++)
		rcCode[i] = eeprom_read_byte((uint8_t*)EEPROM_RC5_CMD + i);

	return;
}

void inputInit(void)
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

	/* Set timer prescaller to 128 (125 kHz) and reset on match*/
	TCCR2 = ((1<<CS22) | (0<<CS21) | (1<<CS20) | (1<<WGM21));
	OCR2 = 125;										/* 125000/125 => 1000 polls/sec */
	TCNT2 = 0;										/* Reset timer value */
	TIMSK |= (1<<OCIE2);							/* Enable timer compare match interrupt */

	rc5CodesInit();

	encRes = eeprom_read_byte((uint8_t*)EEPROM_ENC_RES);
	silenceTime = eeprom_read_byte((uint8_t*)EEPROM_SILENCE_TIMER);

	encCnt = 0;
	cmdBuf = CMD_RC5_END;
	sensTimer = 0;

	return;
}

static uint8_t rc5CmdIndex(uint8_t rc5Cmd)
{
	uint8_t i;

	for (i = 0; i < RC5_CMD_COUNT; i++)
		if (rc5Cmd == rcCode[i])
			return i;

	return CMD_RC5_END;
}

ISR (TIMER2_COMP_vect)
{
	static int16_t btnCnt = 0;						/* Buttons press duration value */
	static uint16_t rc5Timer;

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
	if ((encPrev == ENC_0 && encNow == ENC_A) ||
		(encPrev == ENC_A && encNow == ENC_AB) ||
		(encPrev == ENC_AB && encNow == ENC_B) ||
		(encPrev == ENC_B && encNow == ENC_0))
		encCnt++;
	if ((encPrev == ENC_0 && encNow == ENC_B) ||
		(encPrev == ENC_B && encNow == ENC_AB) ||
		(encPrev == ENC_AB && encNow == ENC_A) ||
		(encPrev == ENC_A && encNow == ENC_0))
		encCnt--;
	encPrev = encNow;								/* Save current encoder state */

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
				case BTN_13:
					cmdBuf = CMD_BTN_13_LONG;
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
	btnSaveBuf = btnNow;

	/* Place RC5 event to command buffer if enough RC5 timer ticks */
	uint16_t rc5Buf = getRC5RawBuf();
	if (rc5Buf != RC5_BUF_EMPTY)
		rc5SaveBuf = rc5Buf;

	static uint8_t togBitNow = 0;
	static uint8_t togBitPrev = 0;

	uint8_t rc5CmdBuf = CMD_RC5_END;
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

	if (cmdBuf == CMD_RC5_END)
		cmdBuf = rc5CmdBuf;

	/* Timer of current display mode */
	if (displayTime)
		displayTime--;

	/* Time from last IR command */
	if (rc5Timer < 1000)
		rc5Timer++;


	if (secTimer) {
		secTimer--;
	} else {
		secTimer = 1000;
		/* Timer of standby mode */
		if (stbyTimer >= 0)
			stbyTimer--;
		/* Silence timer */
		if (silenceTimer >= 0)
			silenceTimer--;
		/* Timer of temperature measurement */
		if (sensTimer)
			sensTimer--;
	}

	/* Timer clock update */
	if (clockTimer)
		clockTimer--;


	return;
};


int8_t getEncoder(void)
{
	int8_t ret = 0;

	if (encRes > 0) {
		while (encCnt >= encRes) {
			ret++;
			encCnt -= encRes;
		}
		while (encCnt <= -encRes) {
			ret--;
			encCnt += encRes;
		}
	} else {
		while (encCnt <= encRes) {
			ret++;
			encCnt -= encRes;
		}
		while (encCnt >= -encRes) {
			ret--;
			encCnt += encRes;
		}
	}

	return ret;
}

cmdID getBtnCmd(void)
{
	cmdID ret = cmdBuf;
	cmdBuf = CMD_RC5_END;
	return ret;
}

uint16_t getRC5Buf(void)
{
	return rc5SaveBuf;
}

uint16_t getBtnBuf(void)
{
	return btnSaveBuf;
}

uint16_t getEncBuf(void)
{
	return encPrev;
}

void setRC5Buf(uint8_t addr, cmdID cmd)
{
	rc5SaveBuf &= (RC5_STBT_MASK | RC5_TOGB_MASK);
	rc5SaveBuf |= ((addr<<6) | cmd);

	return;
}

void setDisplayTime(uint16_t value)
{
	displayTime = value;

	return;
}

uint16_t getDisplayTime(void)
{
	return displayTime;
}

uint8_t getSensTimer(void)
{
	return sensTimer;
}

void setSensTimer(uint8_t val)
{
	sensTimer = val;

	return;
}

int16_t getStbyTimer(void)
{
	return stbyTimer;
}

void setStbyTimer(int16_t val)
{
	stbyTimer = val;

	return;
}

void setSecTimer(uint16_t val)
{
	secTimer = val;

	return;
}

int16_t getSecTimer(void)
{
	return secTimer;
}

void setClockTimer(uint8_t value)
{
	clockTimer = value;

	return;
}

uint8_t getClockTimer(void)
{
	return clockTimer;
}

void enableSilenceTimer(void)
{
	if (silenceTime)
		silenceTimer = 60 * silenceTime;
	else
		silenceTimer = STBY_TIMER_OFF;

	return;
}

int16_t getSilenceTimer(void)
{
	return silenceTimer;
}

void disableSilenceTimer(void)
{
	silenceTimer = STBY_TIMER_OFF;
}
