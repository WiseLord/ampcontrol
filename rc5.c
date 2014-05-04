#include "rc5.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "ks0108.h"

#define CMD_COUNT	16

volatile uint8_t rc5DeviceAddr;

volatile uint8_t rc5Cnt;
volatile uint16_t rc5Cmd;
volatile rc5State state = STATE_BEGIN;

const uint8_t trans[4] = {0x01, 0x91, 0x9b, 0xfb};

volatile uint16_t rc5Buf = 0;

static void rc5Reset()
{
	rc5Cnt = 14;
	rc5Cmd = 0;
	state = STATE_BEGIN;
}

void rc5Init()
{
	MCUCR |= (1<<ISC10);	/* Set INT1 to trigger on any edge */
	RC5_DDR &= ~RC5_DATA;	/* Set PD3 (INT1) to input */
	TCCR1A = 0;				/* Reset Timer1 counter */
	TCCR1B = (1<<CS11);		/* Set Timer1 prescaler to 8 (2MHz) */
	GICR |= (1<<INT1);		/* Enable INT1 interrupt */

	/* Load RC5 device address and from eeprom */
	rc5DeviceAddr = eeprom_read_byte(eepromRC5Addr);
	rc5Reset();
}

ISR(INT1_vect)
{
	uint16_t delay = TCNT1;

	/* event: 0 / 2 - short space/pulse, 4 / 6 - long space/pulse */
	uint8_t event = (RC5_PIN & RC5_DATA) ? 2 : 0;

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
		rc5Buf = rc5Cmd;
		rc5Reset();
	}

	TCNT1 = 0;
}

void showRC5Info()
{
	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(0, 0);
	gdWriteString((uint8_t*)"== RC5 test mode ==");
	gdSetXY(0, 2);
	gdWriteString((uint8_t*)"Raw = ");
	gdWriteString(mkNumString(rc5Buf, 14, '0', 2));
	gdSetXY(0, 4);
	gdWriteString((uint8_t*)"Tog. bit = ");
	gdWriteString(mkNumString(((rc5Buf & 0x0800) > 0), 1, '0', 16));
	gdSetXY(0, 5);
	gdWriteString((uint8_t*)"RC code = ");
	gdWriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	gdSetXY(0, 6);
	gdWriteString((uint8_t*)"Command = ");
	gdWriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));
}
