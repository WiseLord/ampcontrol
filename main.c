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

	uint8_t command = CMD_EMPTY;
	int8_t encCnt = 0;

	spMode  = eeprom_read_byte(eepromSpMode);

	loadParams();
	muteVolume();

	uint8_t bufFM[5];
	uint32_t freqFM = 99500000;

	tea5767SetFreq(freqFM);

	while (1) {
		encCnt = getEncCnt();
		command = getCommand();
		clearCommand();

		/* Handle command */
		switch (command) {
		case CMD_BTN_1:
			switch (dispMode) {
			case MODE_STANDBY:
				powerOn();
				dispMode = MODE_SPECTRUM;
				break;
			case MODE_TEST:
				setDisplayTime(20);
				break;
			default:
				powerOff();
				dispMode = MODE_STANDBY;
				break;
			}
			break;
		case CMD_BTN_2:
			switch (dispMode) {
			case MODE_GAIN:
				nextChan();
			case MODE_SPECTRUM:
			case MODE_TIME:
			case MODE_VOLUME:
			case MODE_BASS:
			case MODE_MIDDLE:
			case MODE_TREBLE:
			case MODE_PREAMP:
			case MODE_BALANCE:
				curSndParam = &gain[chan];
				dispMode = MODE_GAIN;
				setDisplayTime(3);
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
			switch (dispMode) {
			case MODE_VOLUME:
				curSndParam = &bass;
			case MODE_BASS:
				switch (audioProc) {
				case TDA7313_IC:
				case TDA7318_IC:
					curSndParam = &treble;
					dispMode = MODE_TREBLE;
					break;
				case TDA7439_IC:
					curSndParam = &middle;
					dispMode = MODE_MIDDLE;
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
			case MODE_BALANCE:
			case MODE_SPECTRUM:
			case MODE_TIME:
			case MODE_GAIN:
				curSndParam = &volume;
				dispMode = MODE_VOLUME;
				break;
			}
			setDisplayTime(3);
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
			case MODE_STANDBY:
				break;
			default:
				dispMode = MODE_FM_RADIO;
				setDisplayTime(10);
				break;
			}
			break;
		case CMD_ENC:
			switch (dispMode) {
			case MODE_SPECTRUM:
			case MODE_TIME:
				curSndParam = &volume;
				dispMode = MODE_VOLUME;
			case MODE_VOLUME:
			case MODE_BASS:
			case MODE_MIDDLE:
			case MODE_TREBLE:
			case MODE_PREAMP:
			case MODE_BALANCE:
			case MODE_GAIN:
				changeParam(curSndParam, encCnt);
				setDisplayTime(2);
				break;
			case MODE_TIME_EDIT:
				changeTime(encCnt);
				setDisplayTime(20);
				break;
			}
			break;
		}

		/* Exit to default mode and save params to EEPROM*/
		if (getDisplayTime() == 0 && dispMode != MODE_STANDBY) {
			if (dispModePrev != MODE_SPECTRUM) {
				gdSetXY(0, 0);
				eeprom_write_byte(eepromSpMode, spMode);
				_delay_ms(1000);
				saveParams();
				eeprom_write_byte(eepromSpMode, spMode);
			}
			dispMode = MODE_SPECTRUM;
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			gdFill(0x00);

		/* Show things */
		switch (dispMode) {
		case MODE_STANDBY:
			showTime();
			break;
		case MODE_SPECTRUM:
			spBuf = getSpData();
			gdSpectrum32(spBuf, spMode);
			break;
		case MODE_VOLUME:
		case MODE_BASS:
		case MODE_MIDDLE:
		case MODE_TREBLE:
		case MODE_PREAMP:
		case MODE_BALANCE:
		case MODE_GAIN:
			showParam(curSndParam);
			break;
		case MODE_FM_RADIO:
			tea5767ReadStatus(bufFM);
			freqFM = tea5767FreqAvail(bufFM);
			showRadio(bufFM);
			if (tea5767Ready(bufFM))
				fineTune(&freqFM, bufFM);
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;
	}

	return 0;
}
