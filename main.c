#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "eeprom.h"
#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"
#include "rc5.h"
#include "i2c.h"
#include "audio.h"
#include "ds1307.h"
#include "tea5767.h"

#define DISPLAY_TIME_TEST		20
#define DISPLAY_TIME_GAIN		3
#define DISPLAY_TIME_TIME		3
#define DISPLAY_TIME_TIME_EDIT	10
#define DISPLAY_TIME_FM_RADIO	10

enum {
	MODE_STANDBY,
	MODE_SPECTRUM,
	MODE_FM_RADIO,
	MODE_VOLUME,
	MODE_BASS,
	MODE_MIDDLE,
	MODE_TREBLE,
	MODE_PREAMP,
	MODE_GAIN,
	MODE_BALANCE,
	MODE_TIME,
	MODE_TIME_EDIT,
	MODE_MUTE,
	MODE_LOUDNESS,
	MODE_TEST
};

uint8_t spMode;

void switchSpMode()
{
	if (spMode == SP_MODE_STEREO)
		spMode = SP_MODE_MIXED;
	else
		spMode = SP_MODE_STEREO;
	return;
}

void hwInit(void)	/* Hardware initialization */
{
	_delay_ms(100);
	gdInit();		/* Graphic display */
	rc5Init();		/* IR Remote control */
	adcInit();		/* Analog-to-digital converter */
	btnInit();		/* Buttons/encoder polling */
	I2CInit();		/* I2C bus */
	tea5767Init();
	SMF_DDR |= (STDBY | FAN);
	SMF_PORT &= ~(STDBY | MUTE | FAN);
	gdLoadFont(font_ks0066_ru_08, 1);
	sei();
	return;
}

void showRC5Info(uint16_t rc5Buf)
{
	gdLoadFont(font_ks0066_ru_08, 1);
	gdSetXY(0, 0);
	gdWriteString((uint8_t*)"RC5 command");
	gdSetXY(5, 1);
	gdWriteString((uint8_t*)"Raw = ");
	gdWriteString(mkNumString(rc5Buf, 14, '0', 2));
	gdSetXY(5, 2);
	gdWriteString((uint8_t*)"Tog = ");
	gdWriteString(mkNumString(((rc5Buf & 0x0800) > 0), 1, '0', 16));
	gdSetXY(5, 3);
	gdWriteString((uint8_t*)"Adr = ");
	gdWriteString(mkNumString((rc5Buf & 0x07C0)>>6, 2, '0', 16));
	gdSetXY(5, 4);
	gdWriteString((uint8_t*)"Cmd = ");
	gdWriteString(mkNumString(rc5Buf & 0x003F, 2, '0', 16));
	gdSetXY(0, 6);
	gdWriteString((uint8_t*)"Buttons/Encoder");
	gdSetXY(5, 7);
	gdWriteString(mkNumString(BTN_PIN, 8, '0', 2));
	setBacklight(backlight);
}

void showRadio(uint8_t *buf)
{
	uint32_t freq = tea5767FreqAvail(buf);
	gdLoadFont(font_ks0066_ru_24, 1);
	gdSetXY(0, 0);
	gdWriteString((uint8_t*)"FM ");
	gdWriteString(mkNumString(freq/1000000, 3, ' ', 10));
	gdWriteChar('\x7F');
	gdWriteChar('.');
	gdWriteChar('\x7F');
	gdWriteString(mkNumString(freq/100000%10, 1, ' ', 10));
	gdLoadFont(font_ks0066_ru_08, 1);
//	showBar(0, 16, tea5767ADCLevel(buf));
}

void powerOn(void)
{
	SMF_PORT |= STDBY;
	_delay_ms(50);
	SMF_DDR |= MUTE;
	SMF_PORT |= MUTE;
	SMF_PORT |= FAN;
	setBacklight(backlight);
	unmuteVolume();
}

void powerOff(void)
{
	SMF_DDR &= ~MUTE;
	SMF_PORT &= ~MUTE;
	_delay_ms(50);
	SMF_PORT &= ~STDBY;
	SMF_PORT &= ~FAN;
	GD_BACKLIGHT_PORT &= ~GD_BCKL;
	muteVolume();
	saveParams();
	eeprom_write_byte(eepromSpMode, spMode);
}

int main(void)
{
	hwInit();

	uint8_t dispMode = MODE_STANDBY;
	uint8_t dispModePrev = MODE_STANDBY;

	uint8_t *spBuf;
	sndParam *curSndParam = &volume;

	int8_t encCnt = 0;
	uint8_t cmd = CMD_EMPTY;
	uint16_t rc5Buf = RC5_BUF_EMPTY;
	uint16_t rc5BufPrev = RC5_BUF_EMPTY;

	spMode  = eeprom_read_byte(eepromSpMode);

	loadParams();
	muteVolume();

	uint8_t bufFM[5];
	uint32_t freqFM = 99500000;

	tea5767SetFreq(freqFM);

	while (1) {
		encCnt = getEncoder();
		cmd = getBtnCmd();
		rc5Buf = getRC5Buf();

		/* Don't handle any command in test mode */
		if (dispMode == MODE_TEST) {
			if (cmd != CMD_EMPTY)
				setDisplayTime(DISPLAY_TIME_TEST);
			cmd = CMD_EMPTY;
		}

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
		case CMD_RC5_STBY:
			switch (dispMode) {
			case MODE_STANDBY:
				powerOn();
				dispMode = MODE_SPECTRUM;
				break;
			default:
				powerOff();
				dispMode = MODE_STANDBY;
				break;
			}
			break;
		case CMD_BTN_2:
		case CMD_RC5_NEXT_INPUT:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_GAIN:
				nextChan();
			default:
				curSndParam = &gain[chan];
				dispMode = MODE_GAIN;
				setDisplayTime(DISPLAY_TIME_GAIN);
				break;
			}
			break;
		case CMD_BTN_3:
			switch (dispMode) {
			case MODE_FM_RADIO:
				tea5767SetFreq(freqFM - 100000);
				setDisplayTime(10);
				break;
			}
			break;
		case CMD_BTN_4:
			switch (dispMode) {
			case MODE_FM_RADIO:
				tea5767SetFreq(freqFM + 100000);
				setDisplayTime(10);
				break;
			}
			break;
		case CMD_BTN_5:
		case CMD_RC5_MENU:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_VOLUME:
				curSndParam = &bass;
				dispMode = MODE_BASS;
				break;
			case MODE_BASS:
				switch (audioProc) {
				case TDA7439_IC:
					curSndParam = &middle;
					dispMode = MODE_MIDDLE;
					break;
				default:
					curSndParam = &treble;
					dispMode = MODE_TREBLE;
					break;
				}
				break;
			case MODE_MIDDLE:
				curSndParam = &treble;
				dispMode = MODE_TREBLE;
				break;
			case MODE_TREBLE:
				curSndParam = &preamp;
				dispMode = MODE_PREAMP;
				break;
			case MODE_PREAMP:
				curSndParam = &balance;
				dispMode = MODE_BALANCE;
				break;
			default:
				curSndParam = &volume;
				dispMode = MODE_VOLUME;
				break;
			}
			setDisplayTime(3);
			break;
		case CMD_BTN_1_LONG:
		case CMD_RC5_BACKLIGHT:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			default:
				switchBacklight();
			}
			break;
		case CMD_BTN_2_LONG:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			default:
				if (chan == chanCnt - 1) {
					dispMode = MODE_FM_RADIO;
					setDisplayTime(10);
					break;
				}
			}
			break;
		case CMD_BTN_3_LONG:
			switch (dispMode) {
			case MODE_FM_RADIO:
				tea5767Search(freqFM, bufFM, SEARCH_DOWN);
				setDisplayTime(10);
				break;
			}
			break;
		case CMD_BTN_4_LONG:
			switch (dispMode) {
			case MODE_FM_RADIO:
				tea5767Search(freqFM, bufFM, SEARCH_UP);
				setDisplayTime(10);
				break;
			}
			break;
		case CMD_BTN_5_LONG:
			switch (dispMode) {
			}
			break;
		case CMD_BTN_TESTMODE:
			switch (dispMode) {
			case MODE_STANDBY:
				dispMode = MODE_TEST;
				setDisplayTime(20);
				break;
			}
			break;
		case CMD_RC5_MUTE:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			default:
				switchMute();
				dispMode = MODE_MUTE;
				setDisplayTime(2);
				break;
			}
			break;
		case CMD_RC5_LOUDNESS:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			default:
				if (audioProc == TDA7313_IC) {
					switchLoudness();
					dispMode = MODE_LOUDNESS;
					setDisplayTime(2);
				}
				break;
			}
			break;
		case CMD_RC5_INPUT_0:
		case CMD_RC5_INPUT_1:
		case CMD_RC5_INPUT_2:
		case CMD_RC5_INPUT_3:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			default:
				if (dispMode != MODE_FM_RADIO &&
				    chan == chanCnt - 1 && cmd == CMD_RC5_INPUT_0 + chan) {
					curSndParam = &volume;
					dispMode = MODE_FM_RADIO;
					setDisplayTime(DISPLAY_TIME_FM_RADIO);
				} else {
					setChan(cmd - CMD_RC5_INPUT_0);
					curSndParam = &gain[chan];
					dispMode = MODE_GAIN;
					setDisplayTime(DISPLAY_TIME_GAIN);
				}
				break;
			}
			break;
		case CMD_RC5_TIME:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TIME:
			case MODE_TIME_EDIT:
				editTime();
				dispMode = MODE_TIME_EDIT;
				setDisplayTime(DISPLAY_TIME_TIME_EDIT);
				if (isETM())
					setDisplayTime(DISPLAY_TIME_TIME);
				break;
			default:
				stopEditTime();
				dispMode = MODE_TIME;
				setDisplayTime(DISPLAY_TIME_TIME);
				break;
			}
		case CMD_RC5_SP_MODE:
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			default:
				switchSpMode();
				saveParams();
				eeprom_write_byte(eepromSpMode, spMode);
			}
			break;
		}

		/* Emulate RC5 VOL_UP/VOL_DOWN as encoder actions */
		if (cmd == CMD_RC5_VOL_UP)
			encCnt++;
		if (cmd == CMD_RC5_VOL_DOWN)
			encCnt--;

		/* Handle encoder */
		if (encCnt) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEST:
				setDisplayTime(DISPLAY_TIME_TEST);
				break;
			case MODE_TIME_EDIT:
				changeTime(encCnt);
				setDisplayTime(20);
				break;
			case MODE_SPECTRUM:
			case MODE_TIME:
			case MODE_FM_RADIO:
				curSndParam = &volume;
				dispMode = MODE_VOLUME;
			default:
				changeParam(curSndParam, encCnt);
				setDisplayTime(DISPLAY_TIME_GAIN);
				break;
			}
		}

		/* Exit to default mode and save params to EEPROM*/
		if (getDisplayTime() == 0) {
			switch (dispMode) {
			case MODE_STANDBY:
				break;
			case MODE_TEST:
				dispMode = MODE_STANDBY;
				break;
			default:
				if (dispModePrev != MODE_SPECTRUM) {
					gdSetXY(0, 0);
					eeprom_write_byte(eepromSpMode, spMode);
					saveParams();
					eeprom_write_byte(eepromSpMode, spMode);
				}
				dispMode = MODE_SPECTRUM;
				break;
			}
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			gdFill(0x00);

		/* Show things */
		switch (dispMode) {
		case MODE_STANDBY:
			showTime();
			if (dispModePrev == MODE_TEST)
				setBacklight(0);
			break;
		case MODE_TEST:
			showRC5Info(rc5Buf);
			if (rc5Buf != rc5BufPrev)
				setDisplayTime(20);
			break;
		case MODE_SPECTRUM:
			spBuf = getSpData();
			gdSpectrum32(spBuf, spMode);
			break;
		case MODE_FM_RADIO:
			tea5767ReadStatus(bufFM);
			freqFM = tea5767FreqAvail(bufFM);
			showRadio(bufFM);
			if (tea5767Ready(bufFM))
				fineTune(&freqFM, bufFM);
			break;
		case MODE_MUTE:
			showBoolParam(mute, muteLabel);
			break;
		case MODE_LOUDNESS:
			showBoolParam(!loud, loudnessLabel);
			break;
		case MODE_TIME:
		case MODE_TIME_EDIT:
			showTime(0);
			break;
		default:
			showParam(curSndParam);
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;
		/* Save current RC5 raw buf */
		rc5BufPrev = rc5Buf;
	}

	return 0;
}
