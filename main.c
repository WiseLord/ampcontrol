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




	uint16_t rc5Buf = RC5_BUF_EMPTY;

	uint8_t defMode = MODE_SPECTRUM;
	uint8_t stdby = 1;

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
				if (dispMode == MODE_GAIN)
					nextChan();
				curSndParam = &gain[chan];
				dispMode = MODE_GAIN;
				setDisplayTime(3);
				break;
			}
			break;
		case CMD_BTN_5:
			switch (dispMode) {
			case MODE_VOLUME:
				curSndParam = &bass;
				dispMode = MODE_BASS;
				break;
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
//				if (encCnt > 0)
//					for (i = 0; i < encCnt; i++)
//						incParam(curSndParam);
//				else
//					for (i = 0; i < -encCnt; i++)
//						decParam(curSndParam);
				setDisplayTime(2);
				break;
			case MODE_TIME_EDIT:
				changeTime(encCnt);
				setDisplayTime(20);
				break;
			}
			break;
		}

		/* Reset active display mode when timer == 0 */
		if (getDisplayTime() == 0 && dispMode != MODE_STANDBY) {
			dispMode = MODE_SPECTRUM;
			curSndParam = &volume;
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
		}

		/* Save current mode */
		dispModePrev = dispMode;
	}

	while (1) {
		command = getCommand();
		clearCommand();
		encCnt = getEncCnt();

		if (stdby) {										// Standby mode
			if (dispMode == MODE_TEST) {
				if (rc5Buf != getRC5Buf() ||
					~BTN_PIN & BTN_MASK ||
					~ENC_PIN & ENC_AB)
					setDisplayTime(10);
				rc5Buf = getRC5Buf();
				showRC5Info(rc5Buf);
				if (!getDisplayTime()) {
					gdFill(0x00);
					dispMode = MODE_TIME;
				}
			} else {
				setDisplayTime(200);
				showTime();
				switch (command) {
				case CMD_TESTMODE:
					if (dispMode != MODE_TEST)
						gdFill(0x00);
					dispMode = MODE_TEST;
					setDisplayTime(10);
					break;
				case CMD_STBY:
					stdby = 0;
					SMF_PORT |= STDBY;
					_delay_ms(50);
					SMF_DDR |= MUTE;
					SMF_PORT |= MUTE;
					SMF_PORT |= FAN;
					setBacklight(backlight);
					unmuteVolume();
					dispMode = defMode;
					break;
				}
			}
		} else if (command == CMD_EMPTY &&
				   !getDisplayTime()) {						/* Inactive mode */
			if (dispMode != defMode) {
				gdFill(0x00);
				dispMode = defMode;
				saveParams();
				eeprom_write_byte(eepromSpMode, spMode);
				stopEditTime();
			}
			if (dispMode == MODE_SPECTRUM) {
				spBuf = getSpData();
				gdSpectrum32(spBuf, spMode);
			} else {
				showTime();
			}
		} else {											/* Active mode */
			/* Change current mode */
			switch (command) {
			case CMD_MENU:
				setDisplayTime(3);
				switch (dispMode) {
				case MODE_VOLUME:
					if (dispMode != MODE_BASS)
						gdFill(0x00);
					dispMode = MODE_BASS;
					curSndParam = &bass;
					break;
				case MODE_BASS:
					switch (audioProc) {
					case TDA7313_IC:
					case TDA7318_IC:
						if (dispMode != MODE_TREBLE)
							gdFill(0x00);
						dispMode = MODE_TREBLE;
						curSndParam = &treble;
						break;
					default:
						if (dispMode != MODE_MIDDLE)
							gdFill(0x00);
						dispMode = MODE_MIDDLE;
						curSndParam = &middle;
						break;
					}
					break;
				case MODE_MIDDLE:
					if (dispMode != MODE_TREBLE)
						gdFill(0x00);
					dispMode = MODE_TREBLE;
					curSndParam = &treble;
					break;
				case MODE_TREBLE:
					if (dispMode != MODE_PREAMP)
						gdFill(0x00);
					dispMode = MODE_PREAMP;
					curSndParam = &preamp;
					break;
				case MODE_PREAMP:
					if (dispMode != MODE_BALANCE)
						gdFill(0x00);
					dispMode = MODE_BALANCE;
					curSndParam = &balance;
					break;
				case MODE_BALANCE:
				case MODE_SPECTRUM:
				case MODE_TIME:
				case MODE_GAIN:
					if (dispMode != MODE_VOLUME)
						gdFill(0x00);
					dispMode = MODE_VOLUME;
					curSndParam = &volume;
					break;
				default:
					break;
				}
				break;
			case CMD_VOL_UP:
			case CMD_VOL_DOWN:
				setDisplayTime(2);
				switch (dispMode) {
				case MODE_SPECTRUM:
				case MODE_TIME:
					gdFill(0x00);
					dispMode = MODE_VOLUME;
					curSndParam = &volume;
					break;
				default:
					break;
				}
				break;
			case CMD_MUTE:
				setDisplayTime(3);
				tea5767Search(freqFM, bufFM, SEARCH_UP);
				fineTune(&freqFM, bufFM);

//					setDisplayTime(2);
//					if (mode != DISPLAY_MUTE)
//						gdFill(0x00);
//					mode = DISPLAY_MUTE;
				break;
			case CMD_LOUDNESS:
				if (audioProc == TDA7313_IC) {
					setDisplayTime(2);
					if (dispMode != MODE_LOUDNESS)
						gdFill(0x00);
					dispMode = MODE_LOUDNESS;
				}
				break;
			case CMD_TIME:
				setDisplayTime(3);
				tea5767Search(freqFM, bufFM, SEARCH_DOWN);
				fineTune(&freqFM, bufFM);

//					if (mode == DISPLAY_EDIT_TIME)
//						editTime();
//					else {
//						if (mode != DISPLAY_TIME)
//							gdFill(0x00);
//						if (mode == DISPLAY_TIME) {
//							mode = DISPLAY_SPECTRUM;
//							setDisplayTime(0);
//						} else {
//							mode = DISPLAY_TIME;
//						}
//						defMode = mode;
//						stopEditTime();
//					}
				break;
			case CMD_INPUT_0:
			case CMD_INPUT_1:
			case CMD_INPUT_2:
			case CMD_INPUT_3:
				setDisplayTime(3);
				if (dispMode != MODE_GAIN)
					gdFill(0x00);
				dispMode = MODE_GAIN;
				break;
			case CMD_STBY:
				SMF_DDR &= ~MUTE;
				SMF_PORT &= ~MUTE;
				_delay_ms(50);
				stdby = 1;
				SMF_PORT &= ~STDBY;
				SMF_PORT &= ~FAN;
				GD_BACKLIGHT_PORT &= ~GD_BCKL;
				gdFill(0x00);
				dispMode = MODE_TIME;
				muteVolume();
				saveParams();
				eeprom_write_byte(eepromSpMode, spMode);
				break;
			}

			/* Execute command */
			switch (command) {
			case CMD_VOL_UP:
			case CMD_VOL_DOWN:
				switch (dispMode) {
				case MODE_VOLUME:
				case MODE_BASS:
				case MODE_MIDDLE:
				case MODE_TREBLE:
				case MODE_PREAMP:
				case MODE_BALANCE:
				case MODE_GAIN:
					changeParam(curSndParam, encCnt);
					break;
				case MODE_TIME_EDIT:
					setDisplayTime(30);
					changeTime(encCnt);
					break;
				default:
					break;
				}
				break;
			case CMD_MUTE:
//					switchMute();
				break;
			case CMD_LOUDNESS:
//					if (tdaIC == TDA7313_IC) {
//						switchLoudness();
//					}
				break;
			case CMD_BACKLIGHT:
				switchBacklight();
				break;
			case CMD_NEXT_INPUT:
				setDisplayTime(3);
				if (dispMode != MODE_GAIN)
					gdFill(0x00);
				else
					nextChan();
				dispMode = MODE_GAIN;
				curSndParam = &gain[chan];
				break;
			case CMD_EDIT_TIME:
				setDisplayTime(30);
				if (dispMode != MODE_TIME_EDIT)
					gdFill(0x00);
				dispMode = MODE_TIME_EDIT;
				editTime();
				break;
			case CMD_INPUT_0:
				setChan(0);
				curSndParam = &gain[chan];
				break;
			case CMD_INPUT_1:
				setChan(1);
				curSndParam = &gain[chan];
				break;
			case CMD_INPUT_2:
				setChan(2);
				curSndParam = &gain[chan];
				break;
			case CMD_INPUT_3:
				if (audioProc == TDA7439_IC) {
					setChan(3);
					curSndParam = &gain[chan];
				}
				break;
			case CMD_SP_MODE:
				setDisplayTime(100);
				switchSpMode();
				saveParams();
				eeprom_write_byte(eepromSpMode, spMode);
				break;
			}

			/* Show result */
			switch (dispMode) {
			case MODE_VOLUME:
			case MODE_BASS:
			case MODE_MIDDLE:
			case MODE_TREBLE:
			case MODE_PREAMP:
			case MODE_BALANCE:
			case MODE_GAIN:
				showParam(curSndParam);
				break;
			case MODE_TIME:
				setDisplayTime(3);
				showTime();
				break;
			case MODE_TIME_EDIT:
				showTime();
				break;
			case MODE_MUTE:
				showBoolParam(mute, muteLabel);
				break;
			case MODE_LOUDNESS:
				showBoolParam(!loud, loudnessLabel);
				break;
			}
		}
	}

	return 0;
}
