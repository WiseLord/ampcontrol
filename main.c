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

typedef enum {
	DISPLAY_SPECTRUM,
	DISPLAY_VOLUME,
	DISPLAY_BASS,
	DISPLAY_MIDDLE,
	DISPLAY_TREBLE,
	DISPLAY_PREAMP,
	DISPLAY_GAIN,
	DISPLAY_BALANCE,
	DISPLAY_TIME,
	DISPLAY_EDIT_TIME,
	DISPLAY_MUTE,
	DISPLAY_LOUDNESS,
	DISPLAY_TESTMODE
} displayMode;

uint8_t spMode;

void switchSpMode()
{
	if (spMode == MODE_STEREO)
		spMode = MODE_MIXED;
	else
		spMode = MODE_STEREO;
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

int main(void)
{
	hwInit();

	uint8_t *buf;
	uint8_t command = CMD_EMPTY;
	uint8_t cmdCnt = 0;
	uint8_t i;

	displayMode mode = DISPLAY_TIME;
	displayMode defMode = DISPLAY_SPECTRUM;
	uint8_t stdby = 1;

	spMode  = eeprom_read_byte(eepromSpMode);

	sndParam *curParam = &volume;

	loadParams();
	muteVolume();

	uint8_t bufFM[5];
	uint32_t freqFM = 96000000;

	tea5767Search(freqFM, bufFM, SEARCH_UP);

	do {
		tea5767ReadStatus(bufFM);
	} while (!tea5767Ready(bufFM));

	freqFM = tea5767FreqAvail(bufFM);
	tea5767SetOptimalFreq(freqFM);
	tea5767ReadStatus(bufFM);

	while (1) {
		command = getCommand();
		cmdCnt = getCmdCount();

		if (!stdby && (command != CMD_EMPTY || getDisplayTime())) {
			if (mode != DISPLAY_TESTMODE) {
				/* Change current mode */
				switch (command) {
				case CMD_MENU:
					setDisplayTime(3000);
					switch (mode) {
					case DISPLAY_VOLUME:
						if (mode != DISPLAY_BASS)
							gdFill(0x00);
						mode = DISPLAY_BASS;
						curParam = &bass;
						break;
					case DISPLAY_BASS:
						switch (tdaIC) {
						case TDA7313_IC:
						case TDA7318_IC:
							if (mode != DISPLAY_TREBLE)
								gdFill(0x00);
							mode = DISPLAY_TREBLE;
							curParam = &treble;
							break;
						default:
							if (mode != DISPLAY_MIDDLE)
								gdFill(0x00);
							mode = DISPLAY_MIDDLE;
							curParam = &middle;
							break;
						}
						break;
					case DISPLAY_MIDDLE:
						if (mode != DISPLAY_TREBLE)
							gdFill(0x00);
						mode = DISPLAY_TREBLE;
						curParam = &treble;
						break;
					case DISPLAY_TREBLE:
						if (mode != DISPLAY_PREAMP)
							gdFill(0x00);
						mode = DISPLAY_PREAMP;
						curParam = &preamp;
						break;
					case DISPLAY_PREAMP:
						if (mode != DISPLAY_BALANCE)
							gdFill(0x00);
						mode = DISPLAY_BALANCE;
						curParam = &balance;
						break;
					case DISPLAY_BALANCE:
					case DISPLAY_SPECTRUM:
					case DISPLAY_TIME:
					case DISPLAY_GAIN:
						if (mode != DISPLAY_VOLUME)
							gdFill(0x00);
						mode = DISPLAY_VOLUME;
						curParam = &volume;
						break;
					default:
						break;
					}
					break;
				case CMD_VOL_UP:
				case CMD_VOL_DOWN:
					setDisplayTime(2000);
					switch (mode) {
					case DISPLAY_SPECTRUM:
					case DISPLAY_TIME:
						gdFill(0x00);
						mode = DISPLAY_VOLUME;
						curParam = &volume;
						break;
					default:
						break;
					}
					break;
				case CMD_MUTE:
					setDisplayTime(2000);
					if (mode != DISPLAY_MUTE)
						gdFill(0x00);
					mode = DISPLAY_MUTE;
					break;
				case CMD_LOUDNESS:
					if (tdaIC == TDA7313_IC) {
						setDisplayTime(2000);
						if (mode != DISPLAY_LOUDNESS)
							gdFill(0x00);
						mode = DISPLAY_LOUDNESS;
					}
					break;
				case CMD_TIME:
					if (mode == DISPLAY_EDIT_TIME)
						editTime();
					else {
						if (mode != DISPLAY_TIME)
							gdFill(0x00);
						if (mode == DISPLAY_TIME) {
							mode = DISPLAY_SPECTRUM;
							setDisplayTime(0);
						} else {
							mode = DISPLAY_TIME;
						}
						defMode = mode;
						stopEditTime();
					}
					break;
				case CMD_INPUT_0:
				case CMD_INPUT_1:
				case CMD_INPUT_2:
				case CMD_INPUT_3:
					setDisplayTime(3000);
					if (mode != DISPLAY_GAIN)
						gdFill(0x00);
					mode = DISPLAY_GAIN;
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
					mode = DISPLAY_TIME;
					muteVolume();
					saveParams();
					eeprom_write_byte(eepromSpMode, spMode);
					break;
				case CMD_TESTMODE:
					setDisplayTime(20000);
					if (mode != DISPLAY_TESTMODE)
						gdFill(0x00);
					mode = DISPLAY_TESTMODE;
					showRC5Info();
					break;
				default:
					break;
				}

				/* Execute command */
				switch (command) {
				case CMD_VOL_UP:
					for (i = 0; i < cmdCnt; i++)
						switch (mode) {
						case DISPLAY_VOLUME:
						case DISPLAY_BASS:
						case DISPLAY_MIDDLE:
						case DISPLAY_TREBLE:
						case DISPLAY_PREAMP:
						case DISPLAY_BALANCE:
						case DISPLAY_GAIN:
							incParam(curParam);
							break;
						case DISPLAY_EDIT_TIME:
							setDisplayTime(30000);
							incTime();
							break;
						default:
							break;
						}
						break;
				case CMD_VOL_DOWN:
					for (i = 0; i < cmdCnt; i++)
						switch (mode) {
						case DISPLAY_VOLUME:
						case DISPLAY_BASS:
						case DISPLAY_MIDDLE:
						case DISPLAY_TREBLE:
						case DISPLAY_PREAMP:
						case DISPLAY_BALANCE:
						case DISPLAY_GAIN:
							decParam(curParam);
							break;
						case DISPLAY_EDIT_TIME:
							setDisplayTime(30000);
							decTime();
							break;
						default:
							break;
						}
						break;
				case CMD_MUTE:
					switchMute();
					break;
				case CMD_LOUDNESS:
					if (tdaIC == TDA7313_IC) {
						switchLoudness();
					}
					break;
				case CMD_BACKLIGHT:
					switchBacklight();
					break;
				case CMD_NEXT_INPUT:
					setDisplayTime(3000);
					if (mode != DISPLAY_GAIN)
						gdFill(0x00);
					else
						nextChan();
					mode = DISPLAY_GAIN;
					curParam = &gain[chan];
					break;
				case CMD_EDIT_TIME:
					setDisplayTime(30000);
					if (mode != DISPLAY_EDIT_TIME)
						gdFill(0x00);
					mode = DISPLAY_EDIT_TIME;
					editTime();
					break;
				case CMD_INPUT_0:
					setChan(0);
					curParam = &gain[chan];
					break;
				case CMD_INPUT_1:
					setChan(1);
					curParam = &gain[chan];
					break;
				case CMD_INPUT_2:
					setChan(2);
					curParam = &gain[chan];
					break;
				case CMD_INPUT_3:
					if (tdaIC == TDA7439_IC) {
						setChan(3);
						curParam = &gain[chan];
					}
					break;
				case CMD_SP_MODE:
					setDisplayTime(100);
					switchSpMode();
					saveParams();
					eeprom_write_byte(eepromSpMode, spMode);
					break;
				default:
					break;
				}
			}

			/* Show result */
			switch (mode) {
			case DISPLAY_VOLUME:
			case DISPLAY_BASS:
			case DISPLAY_MIDDLE:
			case DISPLAY_TREBLE:
			case DISPLAY_PREAMP:
			case DISPLAY_BALANCE:
			case DISPLAY_GAIN:
				showParam(curParam);
				break;
			case DISPLAY_TIME:
				setDisplayTime(3000);
				showTime();
				break;
			case DISPLAY_EDIT_TIME:
				showTime();
				break;
			case DISPLAY_MUTE:
				showBoolParam(mute, muteLabel);
				break;
			case DISPLAY_LOUDNESS:
				showBoolParam(!loud, loudnessLabel);
				break;
			case DISPLAY_TESTMODE:
//				if (rc5buf) {
					setDisplayTime(20000);
					showRC5Info();
//				}
				break;
			default:
				break;
			}
		} else {
			if (!stdby) {
				if (mode != defMode)
				{
					gdFill(0x00);
					mode = defMode;
					saveParams();
					eeprom_write_byte(eepromSpMode, spMode);
					stopEditTime();
				}
				if (mode == DISPLAY_SPECTRUM) {
					buf = getData();
					gdSpectrum32(buf, spMode);
				} else {
					showTime();
				}
			} else {
				setDisplayTime(200);
				showTime();
				switch (command) {
				case CMD_STBY:
					stdby = 0;
					SMF_PORT |= STDBY;
					_delay_ms(50);
					SMF_DDR |= MUTE;
					SMF_PORT |= MUTE;
					SMF_PORT |= FAN;
					setBacklight(backlight);
					unmuteVolume();
					mode = defMode;
					break;
				default:
					break;
				}
			}
		}
	}

	return 0;
}
